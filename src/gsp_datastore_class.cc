/* -*- mode: cc-mode; tab-width: 2; -*- */

/**
 * @file  gsp_datastore_class.cc
 *
 * @brief DataStore representation
 *
 * @author: Tomasz Gebarowski <gebarowski@gmail.com>
 * @date: Thu Jun 07 18:08:22 2012
 */

#include "gsp_datastore_class.h"
#include "gsp_itemset_class.h"
#include "gsp_sequence_class.h"
#include "gsp_common.h"

/* Documented in header */
GspDataStore::GspDataStore(string file_path)
  : input_file_(file_path.c_str())
{
  line_iterator_ = input_file_;
}

/* Documented in header */
GspDataStore::~GspDataStore()
{
}

/* Documented in header */
bool GspDataStore::RewindStream()
{

  return true;
}

/* Documented in header */
GspSequence *GspDataStore::GetNextSequence()
{
  string id = "", old_id = "";
  GspSequence *sequence = NULL;
  GspItemset * itemset = GetNextItemset(id);

  if (itemset != NULL)
  {
    sequence = new GspSequence(id);
  
    do
    {
      old_id = id;
      sequence->add_itemset(itemset);
      itemset = GetNextItemset(id);
    }
    while (id == old_id &&
           itemset != NULL);
  }
  return sequence;
}


/* Documented in header */
GspItemset * GspDataStore::GetNextItemset(string &p_id)
{
  GspItemset *itemset = NULL;
  istream_iterator<string> eol;
  string id = "";
  
  if (line_iterator_ !=  eol)
  {
    vector<string> *tokens = Tokenize(*line_iterator_, '#');

    if (tokens != NULL &&
        tokens->size() == 3)
    {
      id = (*tokens)[0];

      string timestamp = (*tokens)[1];
      
      vector<string> * seq_tokens = Tokenize((*tokens)[2], ',');

      itemset = new GspItemset(ToInt(timestamp));

      if (seq_tokens != NULL)
      {
        for (unsigned int i = 0; i < seq_tokens->size(); i++)
        {
          itemset->add_item((*seq_tokens)[i]);
        }
        delete seq_tokens;
      }
      delete tokens;
    }

    if (id == p_id ||
        p_id == "")
    {
      ++line_iterator_;
    }
    p_id = id;
  }
  return itemset;
}

