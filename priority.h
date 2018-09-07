// priority.h
//
// PriorityQueue: binary heap priority queue
//
// Copyright 2013 Systems Deployment, LLC
// Author: Morris Bernstein (morris@systems-deployment.com)

#if !defined(CSS343_PRIORITY_H)
#define CSS343_PRIORITY_H

#include <cassert>
#include <cstdlib>
#include <vector>


// This priority queue implementation stores pointers to Things, using
// a Compare function object that returns a numeric value <0, =0, or
// >0 for <, ==, and >.  It is assumed that the Thing class has
// methods int get_priority() and void set_priority(int).  These hold
// indices into the priority queue to simplify implementation of the
// reduce (decrease_key) method.
template<typename Thing, typename Compare>
class PriorityQueue {
public:
	PriorityQueue(Compare compare) : compare_(compare) {}

	// Add a new thing to the priority queue.
	void push_back(Thing* thing);

	// Decrease (raise?) the priority of a given Thing.
	void reduce(Thing* thing);

	// Remove and return the lowest (highest?) priority Thing.  This
	// differs from std::priority_queue which has separate functions.
	// Returns NULL if the queue is empty.
	Thing* pop();

	bool empty() { return data_.empty(); }

	Thing* getElement(int index);
	int getSize() { return data_.size(); }
private:
	void swap(int n1, int n2);

	void sift_up(int n);
	void sift_down(int n);

	Compare compare_;
	std::vector<Thing*> data_;
};


template<typename Thing, typename Compare>
void PriorityQueue<Thing, Compare>::swap(int n1, int n2) {
	Thing* tmp = data_[n1];
	data_[n1] = data_[n2];
	data_[n2] = tmp;
	data_[n1]->set_priority(n1);
	data_[n2]->set_priority(n2);
}


template<typename Thing, typename Compare>
void PriorityQueue<Thing, Compare>::sift_up(int n) {
	if (n == 0) {
		return;
	}

	int parent = (n + 1) / 2 - 1;
	if (compare_(data_[parent], data_[n]) > 0) {
		swap(parent, n);
		sift_up(parent);
	}
}


template<typename Thing, typename Compare>
void PriorityQueue<Thing, Compare>::sift_down(int n) {
	unsigned left = (n + 1) * 2 - 1;
	if (left >= data_.size()) {
		return;
	}
	unsigned right = left + 1;

	if (compare_(data_[n], data_[left]) >= 0) {
		if (right >= data_.size() || compare_(data_[left], data_[right]) <= 0) {
			swap(n, left);
			sift_down(left);
		}
		else {
			swap(n, right);
			sift_down(right);
		}
	}
	else if (right < data_.size() && compare_(data_[n], data_[right]) > 0) {
		swap(n, right);
		sift_down(right);
	}
}


template<typename Thing, typename Compare>
void PriorityQueue<Thing, Compare>::push_back(Thing* thing) {
	int n = data_.size();
	thing->set_priority(n);
	data_.push_back(thing);
	sift_up(n);
}

template<typename Thing, typename Compare>
void PriorityQueue<Thing, Compare>::reduce(Thing* thing) {
	int current_priority = thing->get_priority();
	assert(data_[current_priority] == thing);
	sift_up(current_priority);
}


template<typename Thing, typename Compare>
Thing* PriorityQueue<Thing, Compare>::pop() {
	if (data_.size() == 0) {
		return NULL;
	}
	Thing* min = data_[0];
	data_[0] = *data_.rbegin();
	data_[0]->set_priority(0);
	data_.pop_back();
	sift_down(0);
	return min;
}

template<typename Thing, typename Compare>
inline Thing* PriorityQueue<Thing, Compare>::getElement(int index)
{
	return data_[index];
}



#endif