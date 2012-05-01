/*
 * Copyright (C) 2012 by Glenn Hickey (hickey@soe.ucsc.edu)
 *
 * Released under the MIT license, see LICENSE.txt
 */
#include <cassert>
#include <iostream>
#include <stdexcept>
extern "C" {
#include "cactus.h"
}
#include "cactusDbWrapper.h"

using namespace std;

CactusDbWrapper::CactusDbWrapper() :
  _etree(NULL),
  _flower(NULL),
  _disk(NULL)
{

}

CactusDbWrapper::CactusDbWrapper(const string& dbString) :
  _etree(NULL),
  _flower(NULL),
  _disk(NULL)
{
  open(dbString);
}

CactusDbWrapper::~CactusDbWrapper()
{
  close();
}

void CactusDbWrapper::open(const string& dbString)
{
  close();
  
  stKVDatabaseConf *kvDatabaseConf = 
     stKVDatabaseConf_constructFromString(dbString.c_str());
  if (kvDatabaseConf == NULL)
  {
    throw runtime_error("problem opening " + dbString);
  }
  _disk = cactusDisk_construct(kvDatabaseConf, 0);
  if (_disk == NULL)
  {
    throw runtime_error("problem opening " + dbString);
  }
  _flower = cactusDisk_getFlower(_disk, (Name)0);
  _etree = flower_getEventTree(_flower);
  
  stKVDatabaseConf_destruct(kvDatabaseConf);
}

void CactusDbWrapper::close()
{
  if (_disk != NULL)
  {
    cactusDisk_destruct(_disk);
  }
  _disk = NULL;
  _flower = NULL;
  _etree = NULL;
}

char* CactusDbWrapper::getSequence(const string& eventName,
                                   const string& sequenceName)
{
  assert(_etree != NULL);

  Event* event = eventTree_getEventByHeader(_etree, eventName.c_str());
  Name eventId = event_getName(event);
  
  // todo: ask Ben if there's a faster way
  Flower_SequenceIterator* fit = flower_getSequenceIterator(_flower);
  char* seqString = NULL;
  Sequence* seq = NULL;
  while ((seq = flower_getNextSequence(fit)) != NULL)
  {
    Event* seqEvent = sequence_getEvent(seq);
    Name seqEventId = event_getName(seqEvent);
    const char* seqName = sequence_getHeader(seq);
    if (seqEventId == eventId && sequenceName == string(seqName))
    {
      // todo: verify parameters
      seqString = sequence_getString(seq, 
                                     sequence_getStart(seq),
                                     sequence_getLength(seq),
                                     0);
      break;
    }
  }

  flower_destructSequenceIterator(fit);

  return seqString;
}

// DEBUGGING 
void CactusDbWrapper::printSequenceNames()
{
  // todo: ask Ben if there's a faster way
  Flower_SequenceIterator* fit = flower_getSequenceIterator(_flower);
  Sequence* seq = NULL;
  while ((seq = flower_getNextSequence(fit)) != NULL)
  {
    Event* seqEvent = sequence_getEvent(seq);
    const char* evName = event_getHeader(seqEvent);
    const char* seqName = sequence_getHeader(seq);

    cout << "event:" << evName << "  sequence:" << seqName << endl;
  }

  flower_destructSequenceIterator(fit);
}