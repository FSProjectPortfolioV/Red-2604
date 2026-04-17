struct PSInput
{
    float4 pos : SV_POSITION;
    float brightness : BRIGHTNESS;
};

float4 main(PSInput input) : SV_TARGET
{
    float b = saturate(input.brightness);
    return float4(b, b, b, 1.0f);
}
