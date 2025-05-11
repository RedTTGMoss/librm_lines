#pragma once
namespace RMLinesRenderer {
    template<typename FillFunction>
    struct MonoRaster {
        using Varyings = typename FillFunction::Varyings;

        void operator()(Triangle t, Varyings a, Varyings b, Varyings c);

        FillFunction fill;

        // ********************
        // Internals
        //

        void iterate(float &y, float yMax, float left, float right, float leftInc, float rightInc);
    };
}
