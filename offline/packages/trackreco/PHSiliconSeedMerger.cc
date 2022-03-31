
#include "PHSiliconSeedMerger.h"

#include <fun4all/Fun4AllReturnCodes.h>
#include <phool/PHCompositeNode.h>
#include <phool/getClass.h>
#include <phool/phool.h>
#include <phool/PHObject.h>
#include <phool/PHTimer.h>

#include <trackbase/TrkrDefs.h>

#include <trackbase_historic/SvtxTrackMap.h>
#include <trackbase_historic/SvtxTrack.h>


#include <phool/PHCompositeNode.h>

//____________________________________________________________________________..
PHSiliconSeedMerger::PHSiliconSeedMerger(const std::string &name):
 SubsysReco(name)
{}

//____________________________________________________________________________..
PHSiliconSeedMerger::~PHSiliconSeedMerger()
{
}

//____________________________________________________________________________..
int PHSiliconSeedMerger::Init(PHCompositeNode*)
{

  return Fun4AllReturnCodes::EVENT_OK;
}

//____________________________________________________________________________..
int PHSiliconSeedMerger::InitRun(PHCompositeNode *topNode)
{
  int ret = getNodes(topNode);
  return ret;
}

//____________________________________________________________________________..
int PHSiliconSeedMerger::process_event(PHCompositeNode *)
{

  std::multimap<unsigned int, std::set<TrkrDefs::cluskey>> matches;
  std::set<unsigned int> seedsToDelete;

  for(auto iter1 = m_siliconTrackMap->begin();
      iter1 != m_siliconTrackMap->end(); 
      ++iter1)
    {
      unsigned int track1ID = iter1->first;
      SvtxTrack* track1 = iter1->second;

      if(seedsToDelete.find(track1ID) != seedsToDelete.end())
	{ continue; }

      std::set<TrkrDefs::cluskey> mvtx1Keys;
      for (SvtxTrack::ConstClusterKeyIter iter = track1->begin_cluster_keys();
           iter != track1->end_cluster_keys();
           ++iter)
	{
	  TrkrDefs::cluskey ckey = *iter;
	  if(TrkrDefs::getTrkrId(ckey) == TrkrDefs::TrkrId::mvtxId)
	    { mvtx1Keys.insert(ckey); }
	}

      for(auto iter2 = m_siliconTrackMap->begin();
	  iter2 != m_siliconTrackMap->end();
	  ++iter2) 
	{
	  unsigned int track2ID = iter2->first;
	  if(track1ID == track2ID) { continue; }
	  SvtxTrack* track2 = iter2->second;
	  std::set<TrkrDefs::cluskey> mvtx2Keys;
	  for (SvtxTrack::ConstClusterKeyIter iter = track2->begin_cluster_keys();
	       iter != track2->end_cluster_keys();
	       ++iter)
	    {
	      TrkrDefs::cluskey ckey = *iter;
	      if(TrkrDefs::getTrkrId(ckey) == TrkrDefs::TrkrId::mvtxId)
		{ mvtx2Keys.insert(ckey); }
	    }

	  std::vector<TrkrDefs::cluskey> intersection(mvtx1Keys.size() + mvtx2Keys.size());
	  std::set_intersection(mvtx1Keys.begin(),
				mvtx1Keys.end(),
				mvtx2Keys.begin(),
				mvtx2Keys.end(),
				intersection.begin());

	  if(Verbosity() > 2) 
	    {
	      std::cout << "Track 1 keys " << std::endl;
	      for(auto& key : mvtx1Keys) 
		{ std::cout << "   ckey: " << key << std::endl; }
	      std::cout << "Track 2 keys " << std::endl;
	      for(auto& key : mvtx2Keys) 
		{ std::cout << "   ckey: " << key << std::endl; }
	    }

	  if(intersection.size() > 2) 
	    {
	      for(auto& key : mvtx2Keys)
		{
		  if(mvtx1Keys.find(key) == mvtx1Keys.end())
		    { mvtx1Keys.insert(key); }
		}
	      
	      if(Verbosity() > 2)
		{ 
		  std::cout << "Match IDed"<<std::endl; 
		  for(auto& key : mvtx1Keys)
		    { std::cout << "  total track keys " << key << std::endl; }
		}

	      matches.insert(std::make_pair(track1ID, mvtx1Keys)); 
	      seedsToDelete.insert(track2ID);
	    }
	}
    }

  for(const auto& key : seedsToDelete) 
    {
      m_siliconTrackMap->erase(key);
    }

  if(Verbosity() > 2)
    {
      for(const auto& [key, track] : *m_siliconTrackMap)
	{ track->identify(); }
    }
	  
  return Fun4AllReturnCodes::EVENT_OK;
}

//____________________________________________________________________________..
int PHSiliconSeedMerger::ResetEvent(PHCompositeNode *)
{

  return Fun4AllReturnCodes::EVENT_OK;
}



//____________________________________________________________________________..
int PHSiliconSeedMerger::End(PHCompositeNode *)
{

  return Fun4AllReturnCodes::EVENT_OK;
}

int PHSiliconSeedMerger::getNodes(PHCompositeNode *topNode)
{
  m_siliconTrackMap = findNode::getClass<SvtxTrackMap>(topNode, m_trackMapName.c_str());
  if(!m_siliconTrackMap)
    {
      std::cout << PHWHERE << "No silicon track map, can't merge seeds"
		<< std::endl;
      return Fun4AllReturnCodes::ABORTEVENT;
    }
  
return Fun4AllReturnCodes::EVENT_OK;
}

