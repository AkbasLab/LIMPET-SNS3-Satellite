/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Jani Puttonen <jani.puttonen@magister.fi>
 */

#include <sstream>
#include <dirent.h>
#include <string.h>
#include <errno.h>

#include "ns3/log.h"
#include "ns3/string.h"
#include "ns3/singleton.h"
#include "ns3/satellite-env-variables.h"
#include "satellite-antenna-gain-pattern-container.h"


NS_LOG_COMPONENT_DEFINE ("SatAntennaGainPatternContainer");

const std::string numbers{"0123456789"};

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatAntennaGainPatternContainer);


TypeId
SatAntennaGainPatternContainer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatAntennaGainPatternContainer")
    .SetParent<Object> ()
    .AddConstructor<SatAntennaGainPatternContainer> ()
    .AddAttribute ("PatternsFolder", "Sub-folder in 'antennapatterns' containing the gains definition for each beam",
                   StringValue ("SatAntennaGain72Beams"),
                   MakeStringAccessor (&SatAntennaGainPatternContainer::m_patternsFolder),
                   MakeStringChecker ());
  return tid;
}


TypeId
SatAntennaGainPatternContainer::GetInstanceTypeId () const
{
  NS_LOG_FUNCTION (this);
  return GetTypeId ();
}


SatAntennaGainPatternContainer::SatAntennaGainPatternContainer ()
{
  NS_LOG_FUNCTION (this);

  ObjectBase::ConstructSelf (AttributeConstructionList ());

  std::string dataPath {Singleton<SatEnvVariables>::Get ()->LocateDataDirectory ()};
  std::string patternsFolder = dataPath + "/antennapatterns/" + m_patternsFolder;

  NS_LOG_INFO (this << " directory for antenna patterns set to " << patternsFolder);

  if (!Singleton<SatEnvVariables>::Get ()->IsValidDirectory (patternsFolder))
    {
      NS_FATAL_ERROR (this << " directory " << m_patternsFolder << " not found in antennapatterns folder");
    }

  DIR *dir;
  struct dirent *ent;
  std::string prefix;
  if ((dir = opendir (patternsFolder.c_str ())) != nullptr)
    {
      /* process all the files and directories within patternsFolder */
      while ((ent = readdir (dir)) != nullptr) {
        std::string filename {ent->d_name};
        std::size_t pathLength = filename.length ();
        if (pathLength > 4)
          {
            pathLength -= 4;  // Size of .txt extention
            if (filename.substr (pathLength) == ".txt")
              {
                std::string num, stem = filename.substr (0, pathLength);
                std::size_t found = stem.find_last_not_of (numbers);
                if (found == std::string::npos)
                  {
                    num = stem;
                    stem.erase (0);
                  }
                else
                  {
                    num = stem.substr (found + 1);
                    stem.erase (found + 1);
                  }

                if (prefix.empty())
                  {
                    prefix = stem;
                  }

                if (prefix != stem)
                  {
                    NS_FATAL_ERROR (this << " mixing different prefix for antenna pattern names: " << prefix << " and " << stem);
                  }

                std::string filePath = patternsFolder + "/" + filename;
                std::istringstream ss {num};
                uint32_t i;
                ss >> i;
                if (ss.bad ())
                  {
                    NS_FATAL_ERROR (this << " unable to find beam number in " << filePath << " file name");
                  }

                Ptr<SatAntennaGainPattern> gainPattern = CreateObject<SatAntennaGainPattern> (filePath);

                std::pair<std::map<uint32_t, Ptr<SatAntennaGainPattern> >::iterator, bool> ret;
                ret = m_antennaPatternMap.insert (std::pair<uint32_t, Ptr<SatAntennaGainPattern> > (i, gainPattern));

                if (ret.second == false)
                  {
                    NS_FATAL_ERROR (this << " an antenna pattern for beam " << i << " already exists!");
                  }
              }
          }
      }
      closedir (dir);
    }
  else
    {
      /* could not open directory */
      char const* error = strerror(errno);
      NS_FATAL_ERROR (this << " unable to open directory " << m_patternsFolder << ": " << error);
    }
}

SatAntennaGainPatternContainer::~SatAntennaGainPatternContainer ()
{
  NS_LOG_FUNCTION (this);
}

Ptr<SatAntennaGainPattern>
SatAntennaGainPatternContainer::GetAntennaGainPattern (uint32_t beamId) const
{
  NS_LOG_FUNCTION (this << beamId);

  std::map<uint32_t, Ptr<SatAntennaGainPattern> >::const_iterator agp = m_antennaPatternMap.find (beamId);
  if (agp == m_antennaPatternMap.end ())
    {
      NS_FATAL_ERROR ("SatAntennaGainPatternContainer::GetAntennaGainPattern - unvalid beam id: " << beamId);
    }

  return agp->second;
}

uint32_t
SatAntennaGainPatternContainer::GetBestBeamId (GeoCoordinate coord) const
{
  NS_LOG_FUNCTION (this << coord.GetLatitude () << coord.GetLongitude ());

  double bestGain (-100.0);
  uint32_t bestId (0);

  for (auto const& entry : m_antennaPatternMap)
    {
      uint32_t i = entry.first;
      double gain = entry.second->GetAntennaGain_lin (coord);

      // The antenna pattern has returned a NAN gain. This means
      // that this position is not valid. Return 0, which is not a valid beam id.
      if (std::isnan (gain))
        {
          NS_FATAL_ERROR (this << " returned a NAN antenna gain value!");
        }
      else if (gain > bestGain)
        {
          bestGain = gain;
          bestId = i;
        }
    }

  return bestId;
}

uint32_t
SatAntennaGainPatternContainer::GetNAntennaGainPatterns () const
{
  // Note, that now we assume that all the antenna patterns are created
  // regardless of how many beams are actually simulated.
  return m_antennaPatternMap.size ();
}

} // namespace ns3
