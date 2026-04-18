struct VSInput
{
    float4 pos : POSITION;
    float brightness : BRIGHTNESS;
    int layer : LAYER;
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

    float size = 2.0f; // default

    if (input.layer == 0)
        size = 1.5f; // background
    if (input.layer == 1)
        size = 2.5f; // mid
    if (input.layer == 2)
        size = 4.0f; // foreground

    output.gl_PointSize = size;


    return output;
}