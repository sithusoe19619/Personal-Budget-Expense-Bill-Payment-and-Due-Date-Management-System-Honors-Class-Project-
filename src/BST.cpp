#include "BST.h"

BST::BST() : root(nullptr) {}

BST::~BST() {
    destroyTree(root);
}

void BST::destroyTree(BSTNode* node) {
    if (node == nullptr) return;
    destroyTree(node->left);
    destroyTree(node->right);
    delete node;
}

BST::BSTNode* BST::insertHelper(BSTNode* node, const Expense& e) {
    if (node == nullptr) {
        return new BSTNode(e);
    }
    if (e.date.isBefore(node->data.date)) {
        node->left = insertHelper(node->left, e);
    } else {
        // Equal dates OR after — go right to preserve duplicates
        node->right = insertHelper(node->right, e);
    }
    return node;
}

void BST::insert(const Expense& e) {
    root = insertHelper(root, e);
}

void BST::inOrderHelper(BSTNode* node, std::vector<Expense>& result) const {
    if (node == nullptr) return;
    inOrderHelper(node->left, result);
    result.push_back(node->data);
    inOrderHelper(node->right, result);
}

std::vector<Expense> BST::inOrder() const {
    std::vector<Expense> result;
    inOrderHelper(root, result);
    return result;
}

void BST::rangeHelper(BSTNode* node, const Date& start, const Date& end, std::vector<Expense>& result) const {
    if (node == nullptr) return;

    // If this node's date is before start, the entire left subtree is also
    // before start — prune it and only descend right.
    if (node->data.date.isBefore(start)) {
        rangeHelper(node->right, start, end, result);
        return;
    }

    // If this node's date is after end, the entire right subtree is also
    // after end — prune it and only descend left.
    if (node->data.date.isAfter(end)) {
        rangeHelper(node->left, start, end, result);
        return;
    }

    // Node is within [start, end]: walk left, collect this node, walk right.
    rangeHelper(node->left, start, end, result);
    if (node->data.date.isBetween(start, end)) {
        result.push_back(node->data);
    }
    rangeHelper(node->right, start, end, result);
}

std::vector<Expense> BST::rangeQuery(const Date& start, const Date& end) const {
    std::vector<Expense> result;
    rangeHelper(root, start, end, result);
    return result;
}

std::vector<Expense> BST::search(const Date& d) const {
    return rangeQuery(d, d);
}
