struct VSInput
{
    float4 pos : POSITION;
    float brightness : BRIGHTNESS;
};

struct VSOutput
{
    float4 pos : SV_POSITION;
    float brightness : BRIGHTNESS;
    [[vk::builtin("PointSize")]]
    float gl_PointSize : POINT_SIZE;
};

VSOutput main(VSInput input)
{
    VSOutput output;
    output.pos = input.pos;
    output.brightness = input.brightness;

    output.gl_PointSize = 2.0f;

    return output;
}