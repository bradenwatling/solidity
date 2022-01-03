#include <libsolidity/lsp/GotoDefinition.h>
#include <libsolidity/lsp/Utils.h>
#include <libsolidity/ast/AST.h>
#include <libsolidity/ast/ASTUtils.h>

#include <fmt/format.h>

#include <memory>
#include <string>
#include <vector>

using std::make_shared;
using std::move;
using std::string;
using std::vector;

using namespace solidity::frontend;
using namespace solidity::langutil;

namespace solidity::lsp
{

void GotoDefinition::operator()(MessageID _id, Json::Value const& _args)
{
	string const uri = _args["textDocument"]["uri"].asString();
	string const sourceUnitName = m_fileRepository.clientPathToSourceUnitName(uri);
	if (!m_fileRepository.sourceUnits().count(sourceUnitName))
		throw HandlerError(_id, ErrorCode::RequestFailed, "Unknown file: " + uri);

	auto const lineColumn = parseLineColumn(_args["position"]);
	if (!lineColumn)
		throw HandlerError(
			_id,
			ErrorCode::RequestFailed,
			fmt::format(
				"Unknown position {line}:{column} in file: {file}",
				fmt::arg("line", lineColumn.value().line),
				fmt::arg("column", lineColumn.value().column),
				fmt::arg("file", sourceUnitName)
			)
		);

	ASTNode const* sourceNode = m_server.requestASTNode(sourceUnitName, lineColumn.value());
	vector<SourceLocation> locations;
	if (auto const* expression = dynamic_cast<Expression const*>(sourceNode))
	{
		// Handles all expressions that can have one or more declaration annotation.
		for (auto const* declaration: allAnnotatedDeclarations(expression))
			if (auto location = declarationPosition(declaration); location.has_value())
				locations.emplace_back(move(location.value()));
	}
	else if (auto const* identifierPath = dynamic_cast<IdentifierPath const*>(sourceNode))
	{
		if (auto const* declaration = identifierPath->annotation().referencedDeclaration)
			if (auto location = declarationPosition(declaration); location.has_value())
				locations.emplace_back(move(location.value()));
	}
	else if (auto const* importDirective = dynamic_cast<ImportDirective const*>(sourceNode))
	{
		auto const& path = *importDirective->annotation().absolutePath;
		if (m_fileRepository.sourceUnits().count(path))
			locations.emplace_back(SourceLocation{0, 0, make_shared<string const>(path)});
	}
	else if (auto const* declaration = dynamic_cast<Declaration const*>(sourceNode))
	{
		if (auto location = declarationPosition(declaration); location.has_value())
			locations.emplace_back(move(location.value()));
	}

	Json::Value reply = Json::arrayValue;
	for (SourceLocation const& location: locations)
		reply.append(toJson(m_server.charStreamProvider(), m_fileRepository, location));
	m_client.reply(_id, reply);
}

}
