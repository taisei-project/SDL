#!/usr/bin/env bash

set -e

# NOTE: fxc is tested on Linux with https://github.com/mozilla/fxc2

which fxc &>/dev/null && HAVE_FXC=1 || HAVE_FXC=0
USE_FXC=${USE_FXC:-HAVE_FXC}

spirv_bundle="spir-v.h"
dxbc50_bundle="dxbc50.h"
dxbc51_bundle="dxbc51.h"
metal_bundle="metal.h"

rm -f "$spirv_bundle"
rm -f "$metal_bundle"

if [ "$USE_FXC" != 0 ]; then
    rm -f "$dxbc50_bundle"
    rm -f "$dxbc51_bundle"
fi

make-header() {
    xxd -i "$1" | sed -e 's/^unsigned /const unsigned /g' > "$1.h"
}

compile-hlsl() {
    local src="$1"
    local profile="$2"
    local output_basename="$3"
    local var_name="$(echo "$output_basename" | sed -e 's/\./_/g')"

    fxc "$src" /E main /T $2 /Fh "$output_basename.h" || exit $?
    sed -i "s/g_main/$var_name/;s/\r//g" "$output_basename.h"
}

for i in *.vert *.frag; do
    spv="$i.spv"
    metal="$i.metal"
    hlsl50="$i.sm50.hlsl"
    dxbc50="$i.sm50.dxbc"
    hlsl51="$i.sm51.hlsl"
    dxbc51="$i.sm51.dxbc"

    glslangValidator -g0 -Os "$i" -V -o "$spv" --quiet

    make-header "$spv"
    echo "#include \"$spv.h\"" >> "$spirv_bundle"

    spirv-cross "$spv" --hlsl --shader-model 50 --hlsl-enable-compat --output "$hlsl50"
    spirv-cross "$spv" --hlsl --shader-model 51 --hlsl-enable-compat --output "$hlsl51"

    if [ "$USE_FXC" != "0" ]; then
        if [ "${i##*.}" == "frag" ]; then
            stage="ps"
        else
            stage="vs"
        fi

        compile-hlsl "$hlsl50" ${stage}_5_0 "$dxbc50"
        compile-hlsl "$hlsl51" ${stage}_5_1 "$dxbc51"

        echo "#include \"$dxbc50.h\"" >> "$dxbc50_bundle"
        echo "#include \"$dxbc51.h\"" >> "$dxbc51_bundle"
    fi

    spirv-cross "$spv" --msl --output "$metal"
    make-header "$metal"
    echo "#include \"$metal.h\"" >> "$metal_bundle"
done
