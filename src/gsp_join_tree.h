
#ifndef GSP_JOIN_TREE_H_
#define GSP_JOIN_TREE_H_

#include <map>
#include <list>
#include <string>

#include "gsp_sequence_class.h"


/**
 * @class GspJoinTree
 *
 * @brief Class providing a suffix tree representation of the sequence set. Last
 * items are dropped from the suffixes to provide an easy way of joinable sequences
 */
class GspJoinTree
{
  public:
    /**
     * @brief Constructs an empty tree
     */
    GspJoinTree();
    /**
     * @brief Destroys a tree
     */
    ~GspJoinTree();

    /**
     * @brief insert a sequence into the tree
     */
    void AddSequence(GspSequence *seq);

    /**
     * @brief finds all joinable sequences of a passed sequence
     */
    void FindJoinable(GspSequence *seq, std::list<GspSequence *> *&result);

  private:
    /**
     * @Class Internal node of a tree
     */
    class Node
    {
      private:
        std::map<std::string, Node *> *nodes_;
        std::list<GspSequence *> *sequences_;

      public:
        /**
         * @brief Construct a node
         */
        Node() : nodes_(NULL), sequences_(NULL)
        {
        }
        /**
         * @brief Destroy a node and all its subnodes
         */
        ~Node()
        {
          delete sequences_;
          if (nodes_)
            for(std::map<std::string, Node *>::iterator it = nodes_->begin(); it != nodes_->end(); ++it)
              delete it->second;
          delete nodes_;
        }

        /**
         * @brief Recursively traverse tree and insert a sequence where it belongs
         */
        void AddSequence(GspSequence *seq, GspSequence::IterType iter);

        /**
         * @brief Recursively traverse a tree and find all the sequences that can be joined with the passed sequence
         */
        void FindJoinable(GspSequence::IterType &current, GspSequence::IterType &final, std::list<GspSequence *> *&result);
    };

    Node *root_;
};

#endif /* GSP_JOIN_TREE_H_ */
