#pragma once
namespace RMLinesRenderer {
    template<typename FillFunction>
    struct LerpRaster {
        using Varyings = typename FillFunction::Varyings;

        void operator()(Triangle t, Varyings a, Varyings b, Varyings c);

        FillFunction fill;
    };
}
