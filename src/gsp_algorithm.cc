/* -*- mode: cc-mode; tab-width: 2; -*- */

/**
 * @file  gsp_algorithm.cc
 *
 * @brief Generalized Sequential Patterns (GSP) algorithm implementation
 *
 * @author: Adam Szczepankiewicz <adam.szczepankiewicz@cern.ch>
 * @date: Sat Jun 9 15:30:39 2012
 */

#include <cstdlib>

#include "gsp_algorithm.h"
#include "gsp_sequence_reader.h"
#include "gsp_sequence_pool.h"

/* Documented in header */
GspAlgorithm::GspAlgorithm(GspSequenceReader *in_reader,
                           unsigned int min_support,
                           int window_size,
                           int min_gap,
                           int max_gap)
  : reader_(in_reader),
    min_support_(min_support),
    window_size_(window_size),
    min_gap_(min_gap),
    max_gap_(max_gap),
    frequent_(NULL),
    candidates_(NULL),
    finished_(false)
{
  std::cout << "GSP (min-support: " << ToString(min_support_)\
            << " window-size: " << ToString(window_size_)\
            << " min-gap: " << ToString(min_gap_)\
            << " max-gap: " << ToString(max_gap_) <<")"<< std::endl;

  frequent_ = new GspSequencePool(reader_.get(),
                                  this);

  if (frequent_->GetSequenceCount() == 0)
  {
    finished_ = true;
  }
}

/* Documented in header */
GspAlgorithm::~GspAlgorithm()
{
  delete frequent_;
  delete candidates_;
}

void GspAlgorithm::PrintResult(std::ostream &str)
{
  frequent_->PrintResult(str);
}

/* Documented in header */
void GspAlgorithm::PrintFrequentSequences()
{
  if(frequent_)
    frequent_->PrintSequences();
}

/* Documented in header */
void GspAlgorithm::PrintCandidateSequences()
{
  if(candidates_)
    candidates_->PrintSequences();
}

/* Documented in header */
void GspAlgorithm::RunPass()
{
  if (finished_)
    return;
  std::cout<<"Looking for "<<frequent_->GetK() + 1<<"-frequent sequences..."<<std::endl;
  std::cout<<"Joining the "<<frequent_->GetK()<<"-frequent set... "<<std::flush;
  candidates_ = frequent_->Join();
  std::cout<<candidates_->GetSequenceCount()<<" "<<candidates_->GetK()<<"-candidates found!"<<std::endl;
  std::cout<<"Removing candidates with nonfrequent contiguous subsequences... "<<std::flush;
  std::set<std::string> *strRep = frequent_->ToStringSet();
  candidates_->DropNonContiguous(strRep);
  std::cout<<candidates_->GetSequenceCount()<<" candidates left!"<<std::endl;
  delete strRep;
  if (candidates_->GetSequenceCount() == 0)
  {
    finished_ = true;
    delete candidates_;
    candidates_ = NULL;

    return;
  }
  std::cout<<"Counting support for candidates... "<<std::flush;
  candidates_->CountSupport(reader_.get());
  std::cout<<"Done!"<<std::endl;
  std::cout<<"Dropping nonfrequent candidates... "<<std::flush;
  candidates_->DropSequences();
  std::cout<<candidates_->GetSequenceCount()<<" frequent "<<candidates_->GetK()<<"-sequenced found!"<<std::endl<<std::endl;

  if (candidates_->GetSequenceCount() == 0)
  {
    finished_ = true;
    delete candidates_;
    candidates_ = NULL;

    return;
  }

  delete frequent_;
  frequent_ = candidates_;
  candidates_ = NULL;
}

