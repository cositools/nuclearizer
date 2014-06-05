/*
 * MNCTModule.cxx
 *
 *
 * Copyright (C) 2008-2008 by Andreas Zoglauer.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Andreas Zoglauer.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */


////////////////////////////////////////////////////////////////////////////////
//
// MNCTModule
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MNCTModule.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MNCTModule)
#endif


////////////////////////////////////////////////////////////////////////////////


MNCTModule::MNCTModule()
{
  // Construct an instance of MNCTModule

  m_Name = "Base class...";
  m_XmlTag = "BaseClass"; // No spaces allowed

  m_HasOptionsGUI = false;
}


////////////////////////////////////////////////////////////////////////////////


MNCTModule::~MNCTModule()
{
  // Delete this instance of MNCTModule
}

////////////////////////////////////////////////////////////////////////////////
MString MNCTModule::Report()
{

  return "";
}
////////////////////////////////////////////////////////////////////////////////


bool MNCTModule::ReadXmlConfiguration(MXmlNode* Node)
{
  //! Read the configuration data from an XML node

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode* MNCTModule::CreateXmlConfiguration() 
{
  //! Create an XML node tree from the configuration

  MXmlNode* Node = new MXmlNode(0, m_XmlTag);

  return Node;
}


// MNCTModule.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
