/*
Copyright (c) 2016 Ryan L. Guy

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgement in the product documentation would be
   appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

#include <utils/stopwatch.h>

///////////////////////////////////////////////////////////////////////

using sw = world_builder::Stopwatch;

///////////////////////////////////////////////////////////////////////

sw::Stopwatch()
  :
  m_is_started(false),
  m_time_begin(0),
  m_time_end(0),
  m_time_running(0.0)
{ }

///////////////////////////////////////////////////////////////////////

void sw::Stopwatch::Start()
{

#if defined(__linux__) || defined(__APPLE__) || defined(__MACOSX)
  struct timeval tp;
  gettimeofday(&tp, nullptr);
  m_time_begin = (double)tp.tv_sec + (double)tp.tv_usec / 1000000.0;
#elif defined(_WIN32)
  m_time_begin = (double)GetTickCount() / 1000.0;
#else
#endif

  m_is_started = true;
}

///////////////////////////////////////////////////////////////////////

void sw::Stopwatch::Stop()
{
  if(!m_is_started)
  {
    return;
  }

#if defined(__linux__) || defined(__APPLE__) || defined(__MACOSX)
  struct timeval tp;
  gettimeofday(&tp, nullptr);
  m_time_end = (double)tp.tv_sec + (double)tp.tv_usec / 1000000.0;
#elif defined(_WIN32)
  m_time_end = (double)GetTickCount() / 1000.0;
#else
#endif

  double time = m_time_end - m_time_begin;
  m_time_running += time;
}

///////////////////////////////////////////////////////////////////////

void sw::Stopwatch::Reset()
{
  m_is_started = false;
  m_time_running = 0.0;
}

///////////////////////////////////////////////////////////////////////

double sw::Stopwatch::Get_time()
{
  return m_time_running >= 0.0 ? m_time_running : 0.0;
}

///////////////////////////////////////////////////////////////////////
