/* -*- mode: cc-mode; tab-width: 2; -*- */

/**
 * @file  gsp_sequence_class.cc
 *
 * @brief Sequence datatype class definition
 *
 * @author: Tomasz Gebarowski <gebarowski@gmail.com>
 * @date: Fri May 18 15:46:11 2012
 */

#include "gsp_itemset_class.h"
#include "gsp_sequence_class.h"
#include "gsp_join_tree.h"

#include <cmath>
#include <sstream>

/* Documented in header */
GspSequence::GspSequence(int id) : id_(id), support_(0)
{
}

/* Documented in header */
GspSequence::GspSequence(const GspSequence &src)
{
  for (std::list<GspItemset *>::const_iterator it = src.itemsets_.begin();
       it != src.itemsets_.end();
       ++it)
  {
    itemsets_.push_back(new GspItemset(**it));
  }
  iter_ = itemsets_.begin();

  support_ = 0;
}


/* Documented in header */
GspSequence::~GspSequence()
{
  /* Remove all itemsets added to this sequence */
  for (std::list<GspItemset *>::iterator it = itemsets_.begin();
       it != itemsets_.end();
       ++it)
  {
    delete *it;
  }
}

/* Documented in header */
GspSequence *GspSequence::JoinSequences(GspSequence *right)
{

  GspSequence *leftCopy = new GspSequence(*this);
  GspSequence *rightCopy = new GspSequence(*right);
  GspSequence *ret = NULL;

  leftCopy->DropFirstItem();
  rightCopy->DropLastItem();

  if (*leftCopy == *rightCopy)
  {
    ret = new GspSequence(*this);
    ret->AppendSequence(right);
  }

  delete leftCopy;
  delete rightCopy;

  return ret;
}

/* Documented in header */
void GspSequence::DropFirstItem()
{
  if (itemsets_.size() > 0)
  {
    GspItemset *firstItemSet = itemsets_.front();
    if (firstItemSet->item_count() == 1)
    {
      delete firstItemSet;
      itemsets_.pop_front();
    }
    else
      firstItemSet->remove_first_item();
  }
}

/* Documented in header */
void GspSequence::DropLastItem()
{
  if (itemsets_.size() > 0)
  {
    GspItemset *lastItemSet = itemsets_.back();
    if (lastItemSet->item_count() == 1)
    {
      delete lastItemSet;
      itemsets_.pop_back();
    }
    else
      lastItemSet->remove_last_item();
  }
}

/* Documented in header */
std::string GspSequence::ToString() const
{
  std::stringstream strStream;

  for(std::list<GspItemset *>::const_iterator it = itemsets_.begin(); it != itemsets_.end(); ++it)
  {
    strStream<<(*it)->ToString();
  }

  return strStream.str();
}

/* Operators */

/* Documented in header */
bool GspSequence::operator==(const GspSequence &other) const
{
  if (this->itemsets_.size() != other.itemsets_.size())
    return false;

  std::list<GspItemset *>::const_iterator
    leftIt = this->itemsets_.begin(),
    rightIt = other.itemsets_.begin();

  while (leftIt != this->itemsets_.end())
  {
    if (!(**leftIt == **rightIt))
      return false;
    ++leftIt;
    ++rightIt;
  }

  return true;
}

/* Documented in header */
void GspSequence::AppendSequence(GspSequence *right)
{
  GspItemset *lastRight = *(--right->itemsets_.end());

  if(lastRight->item_count() == 1)
  {
    this->add_itemset(new GspItemset(*lastRight));
  }
  else
  {
    GspItemset *lastLeft = *(--this->itemsets_.end());
    lastLeft->add_item(*(--lastRight->end()));
  }
}

/* Documented in header */
void GspSequence::CheckCandidates(int windowSize, int minGap, int maxGap)
{
  //build the sorted list representation of the sequence
  OrderedItemMap itemListMap;
  for(GspSequence::IterType it = begin(); it != end(); ++it)
  {
    for (GspItemset::IterType itemIter = (*it)->begin(); itemIter != (*it)->end(); ++itemIter)
    {
      int item = *itemIter;
      if (itemListMap.find(item) != itemListMap.end())
      {
        itemListMap[item].insert(*it);
      }
      else
      {
        std::set<GspItemset *, GspItemsetTimeComparer> timestampList;
        timestampList.insert(*it);
        itemListMap[item] = timestampList;
      }
    }
  }

/*  std::cout<<ToString()<<std::endl<<std::endl;
//  print_candidates();
  std::cout<<"\nMapa:\n";
  for(OrderedItemMap::iterator mapYt = itemListMap.begin(); mapYt != itemListMap.end(); ++mapYt)
  {
    std::cout<<mapYt->first<<": ";
    for(OrderedItemsetSet::iterator setYt = mapYt->second.begin(); setYt != mapYt->second.end(); ++setYt)
      std::cout<<(*setYt)->get_timestamp()<<" ";
    std::cout<<std::endl;
  }
  std::cout<<std::endl<<std::flush;*/

  //for every candidate sequence...
  for(std::set<GspSequence *>::const_iterator candIter = candidates_.begin(); candIter != candidates_.end(); ++candIter)
  {
    GspSequence *candSeq = *candIter;
//    std::cout<<"CAND: "<<candSeq->ToString();
    //build a vector of objects representing the itemsets for the algorithm
    std::vector<ItemSetIterators *> candItemSets;
    candItemSets.clear();
    bool created = true;
    for(GspSequence::IterType itemsetIter = candSeq->begin(); itemsetIter != candSeq->end(); ++itemsetIter)
    {
      //an itemset representation
//      std::cout<<" "<<(*itemsetIter)->ToString();
//      std::cout<<std::endl;
      ItemSetIterators *candStruct = new ItemSetIterators(&itemListMap, *itemsetIter, windowSize);
      //check if all the items in the candidate are present in the client
      if(!candStruct->init())
      {
        //NO! definetly not a subsequence
        delete candStruct;
        created = false;
        break;
      }
      else
      {
//        candStruct->print_content();
//        std::cout<<std::endl;
        candItemSets.push_back(candStruct);
      }
    }
//    std::cout<<std::endl;
    //get number of itemsets
    int maxEl = candItemSets.size();
    //if the representation was correctly created, start matching the items
    if (created)
    {
      //start with the first (earliest) itemset, perform the GSP matching phase
      int current = 0;
      int currentMin = -1;
      int currentTimeEnd, currentTimeStart;

      bool OK = true;
      //until last itemset reached
      while(current != maxEl)
      {
        //move the items in an itemset past the specified minimal timestamp
        if(candItemSets[current]->move(currentMin))
        {
//          std::cout<<" current: "<<current<<std::endl;
          //if possible, find the next mathing that fits in the sliding window
          if(candItemSets[current]->find())
          {
            currentTimeEnd = candItemSets[current]->getMaxTime();
            currentTimeStart = candItemSets[current]->getMinTime();
            if(current == 0
                || (maxGap <= 0) || (currentTimeEnd - candItemSets[current - 1]->getMinTime() <= maxGap))
            {
              //if it was the first itemset or there is no max gap condition or the max
              //gap i satisfied, move forward observing the min gap condition
              currentMin = currentTimeEnd + minGap;
              ++current;
            }
            else
            {
              //the max gap wasn't satisfied, move backwards
              currentMin = currentTimeStart - maxGap;
              --current;
            }
          }
          else
          {
            //no mathing in the sliding window, not a subsequence, break
            OK = false;
          }
        }
        else
        {
          //no values past the specified time, not a subsequence
          OK = false;
        }

        if(!OK)
          break;
      }

      //if mathed, increase a support for the candidate
      if(current == maxEl)
        candSeq->increase_support();
    }
    //delete the itemset representation
    for(int wt = 0; wt < maxEl; ++wt)
      delete candItemSets[wt];
  }
}

bool GspSequence::ContiguousCheck(GspJoinTree *joinTree)
{
  GspSequence::IterType first = begin();
  GspSequence::IterType invalid = end();
  GspSequence::IterType last = --end();

  GspSequence *tempSequence;
//  std::cout<<"Cont "<<ToString()<<std::endl;
  for(GspSequence::IterType itemsetIter = first; itemsetIter != invalid; ++itemsetIter)
  {
    if(itemsetIter == first  && (*itemsetIter)->item_count() == 1)
    {
      tempSequence = new GspSequence(-1);

      GspSequence::IterType temp = first;
      ++temp;
      while(temp !=invalid)
      {
        tempSequence->add_itemset(new GspItemset(**temp));
        ++temp;
      }
//      std::cout<<tempSequence->ToString()<<std::endl;
      if(joinTree->FindSequence(tempSequence) == false)
      {
        delete tempSequence;
        return false;
      }

      delete tempSequence;
    }
    else if (itemsetIter == last && (*itemsetIter)->item_count() == 1)
    {
      tempSequence = new GspSequence(-1);
      GspSequence::IterType temp = first;
      while(temp !=last)
      {
        tempSequence->add_itemset(new GspItemset(**temp));
        ++temp;
      }
//      std::cout<<tempSequence->ToString()<<std::endl;
      if(joinTree->FindSequence(tempSequence) == false)
      {
        delete tempSequence;
        return false;
      }

      delete tempSequence;
    }
    else
    {
      if((*itemsetIter)->item_count() > 1)
      {
        for(GspItemset::IterType exclItemIter = (*itemsetIter)->begin(); exclItemIter != (*itemsetIter)->end(); ++exclItemIter)
        {
          tempSequence = new GspSequence(-1);

          for(GspSequence::IterType tt = first; tt != itemsetIter; ++tt)
          {
            tempSequence->add_itemset(new GspItemset(**tt));
          }

          GspItemset *tempItemset = new GspItemset(**itemsetIter);
          tempItemset->drop_item(*exclItemIter);
          tempSequence->add_itemset(tempItemset);

          GspSequence::IterType tt = itemsetIter;
          ++tt;
          for(;tt != invalid; ++tt)
          {
            tempSequence->add_itemset(new GspItemset(**tt));
          }
//          std::cout<<tempSequence->ToString()<<std::endl;
          if(joinTree->FindSequence(tempSequence) == false)
          {
            delete tempSequence;
            return false;
          }

          delete tempSequence;
        }
      }
    }
  }

  return true;
}

