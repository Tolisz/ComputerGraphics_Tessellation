#include "fpsCounter.h"

#include <cmath>
#include <algorithm>

void fpsCounter::SetTimeInterval(const float& seconds)
{
    m_timeInterval = seconds;
    Reset();
}

void fpsCounter::Update(const float& deltaTime)
{
    if (m_frameCounter >= m_frameToSum) {
        m_avarageRenderTime = m_renderTimeCounter / m_frameToSum;
        m_avarageFPS = 1.0f / m_avarageRenderTime;  
        
        m_frameToSum = static_cast<int>(std::max(std::ceil(m_timeInterval / m_avarageRenderTime), 1.0f));

        m_frameCounter = 0;
        m_renderTimeCounter = 0.0f;
    }
    else {
        m_frameCounter++;
        m_renderTimeCounter += deltaTime;
    }
}   

void fpsCounter::Reset()
{
    m_frameCounter = 1;
    m_frameToSum = 1;
}

float fpsCounter::GetRenderTime() const
{
    return m_avarageRenderTime;
}

float fpsCounter::GetAvarageFPS() const
{
    return m_avarageFPS;
}
