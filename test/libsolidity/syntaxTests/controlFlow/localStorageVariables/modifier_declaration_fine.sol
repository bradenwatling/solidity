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
    function f(bool flag) revertIfNoReturn() internal view {
        if (flag) s;
    }
    function g(bool flag) revertIfNoReturn() ifFlag(flag) internal view {
        s;
    }

}
// ----
// Warning 5740: (247-275): Unreachable code.
// Warning 5740: (328-332): Unreachable code.
// Warning 5740: (348-366): Unreachable code.
