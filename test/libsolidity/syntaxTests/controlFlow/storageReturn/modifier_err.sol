contract C {
    modifier revertIfNoReturn() {
        _;
        revert();
    }
    modifier ifFlag(bool flag) {
        if (flag)
            _;
    }
    struct S { uint a; }
    S s;
    function f(bool flag) ifFlag(flag) internal view returns(S storage) {
        return s;
    }

    function g(bool flag) ifFlag(flag) revertIfNoReturn() internal view returns(S storage) {
        return s;
    }
}
// ----
// Warning 5740: (378-403): Unreachable code.
