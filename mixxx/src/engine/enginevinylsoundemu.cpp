/***************************************************************************
                          enginevinylsoundemu.cpp  -  description
                             -------------------
    copyright            : (C) 2007 by Albert Santoni
    email                : gamegod \a\t users.sf.net
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <QtDebug>
#include <QtGlobal>
#include "configobject.h"
#include "controlobject.h"
#include "enginevinylsoundemu.h"
#include "sampleutil.h"

/** This class emulates the response of a vinyl record's audio to changes
 *   in speed. In practice, it quiets the audio during very slow playback.
 *   This also helps mask the aliasing due to interpolation that occurs at
 *   these slow speeds.
 */

EngineVinylSoundEmu::EngineVinylSoundEmu(ConfigObject<ConfigValue> * pConfig, const char * group)
{
    m_pConfig = pConfig;
    m_pRateEngine = ControlObject::getControl(ConfigKey(group, "rateEngine"));
    m_fSpeed = m_fOldSpeed = 0.0f;
    m_fGainFactor = 1.0f;
}

EngineVinylSoundEmu::~EngineVinylSoundEmu()
{

}

void EngineVinylSoundEmu::process(const CSAMPLE * pIn, const CSAMPLE * pOut, const int iBufferSize)
{
    CSAMPLE * pOutput = (CSAMPLE *)pOut;
    m_fSpeed = (float)m_pRateEngine->get();
    float rateFrac = 2 * (m_fSpeed - m_fOldSpeed) / (float)iBufferSize;
    float curRate = m_fOldSpeed;
    
    const float thresholdSpeed = 0.070f; //Scale volume if playback speed is below 7%.
    
    //iterate over old rate to new rate to prevent audible pops    
    for (int i=0; i<iBufferSize; i+=2)
    {
        if (fabs(curRate) < thresholdSpeed) {
           float dither = (float)(rand() % 32768) / 32768 - 0.5; // dither
           float gainfrac = fabs(curRate) / thresholdSpeed;
           pOutput[i] = gainfrac * (float)pIn[i] + dither;
           pOutput[i+1] = gainfrac * (float)pIn[i+1] + dither;
        }
        else
        {
           pOutput[i] = pIn[i];
           pOutput[i+1] = pIn[i+1];
        }
        curRate += rateFrac;
    }
    m_fOldSpeed = m_fSpeed;
}

