#pragma once

class FortranScriptCaller {
    extern "C" {
        void fortfunc(int *ii, float *ff);
    }

    main() {
        int ii = 5;
        float ff = 5.5;
        fortfunc(&ii, &ff);
        return 0;
    }
};