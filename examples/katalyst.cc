/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 *
 * Author: Ozgur Ural <ozzgural@gmail.com>
 *
 */

#include "ns3/core-module.h"
#include "ns3/config-store-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/satellite-module.h"
#include "ns3/applications-module.h"
#include "ns3/traffic-module.h"

using namespace ns3;

/**
 * \file katalyst.cc
 * \ingroup satellite
 *
 * \brief This file is used to run the Katalyst simulation.
 */

NS_LOG_COMPONENT_DEFINE ("Katalyst");

int
main (int argc, char *argv[])
{
  std::string inputFileNameWithPath = Singleton<SatEnvVariables>::Get ()->LocateDirectory ("contrib/satellite/examples") + "/katalyst-input.xml";

  Ptr<SimulationHelper> simulationHelper = CreateObject<SimulationHelper> ("katalyst-launcher");
  simulationHelper->DisableAllCapacityAssignmentCategories ();
  simulationHelper->EnableCrdsa ();

  CommandLine cmd;
  simulationHelper->AddDefaultUiArguments (cmd, inputFileNameWithPath);
  cmd.Parse (argc, argv);

  simulationHelper->ConfigureAttributesFromFile (inputFileNameWithPath);
  simulationHelper->StoreAttributesToFile ("katalystParametersUsed.xml");
  simulationHelper->RunSimulation ();
}
