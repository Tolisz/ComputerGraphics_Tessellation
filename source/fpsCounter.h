#pragma once

class fpsCounter 
{
private:

    int m_frameToSum = 100;
    int m_frameCounter = 0;
    float m_renderTimeCounter = 0.0f;
    float m_avarageFPS = 0.0f;
    float m_avarageRenderTime = 0.0f;

    float m_timeInterval = 0.5f;

public:

    void SetTimeInterval(const float& seconds);
    void Update(const float& deltaTime);
    void Reset();
    float GetRenderTime() const;
    float GetAvarageFPS() const;
};