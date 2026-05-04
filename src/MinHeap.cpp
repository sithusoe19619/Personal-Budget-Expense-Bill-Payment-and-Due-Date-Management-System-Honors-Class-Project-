#include "MinHeap.h"
#include <cassert>
#include <utility>

MinHeap::MinHeap(int initialCapacity)
    : heap(nullptr), size(0), capacity(initialCapacity) {
    if (capacity < 1) {
        capacity = 1;
    }
    heap = new Bill[capacity];
}

MinHeap::~MinHeap() {
    delete[] heap;
}

void MinHeap::heapifyUp(int index) {
    while (index > 0) {
        int parent = (index - 1) / 2;
        if (heap[index].dueDate.isBefore(heap[parent].dueDate)) {
            std::swap(heap[index], heap[parent]);
            index = parent;
        } else {
            break;
        }
    }
}

void MinHeap::heapifyDown(int index) {
    while (true) {
        int left = 2 * index + 1;
        int right = 2 * index + 2;
        int smallest = index;

        if (left < size && heap[left].dueDate.isBefore(heap[smallest].dueDate)) {
            smallest = left;
        }
        if (right < size && heap[right].dueDate.isBefore(heap[smallest].dueDate)) {
            smallest = right;
        }

        if (smallest != index) {
            std::swap(heap[index], heap[smallest]);
            index = smallest;
        } else {
            break;
        }
    }
}

void MinHeap::insert(const Bill& bill) {
    if (size == capacity) {
        int newCapacity = capacity * 2;
        Bill* newHeap = new Bill[newCapacity];
        for (int i = 0; i < size; ++i) {
            newHeap[i] = heap[i];
        }
        delete[] heap;
        heap = newHeap;
        capacity = newCapacity;
    }

    heap[size] = bill;
    ++size;
    heapifyUp(size - 1);
}

Bill MinHeap::extractMin() {
    assert(size > 0);
    Bill minBill = heap[0];
    heap[0] = heap[size - 1];
    --size;
    if (size > 0) {
        heapifyDown(0);
    }
    return minBill;
}

Bill MinHeap::peek() const {
    assert(size > 0);
    return heap[0];
}

bool MinHeap::isEmpty() const {
    return size == 0;
}

bool MinHeap::markPaidByName(const std::string& name, const Date& paymentDate) {
    for (int i = 0; i < size; ++i) {
        if (heap[i].name == name) {
            heap[i].markAsPaid(paymentDate);
            return true;
        }
    }
    return false;
}
