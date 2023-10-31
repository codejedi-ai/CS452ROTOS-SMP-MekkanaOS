// A C++ program to demonstrate common Binary Heap Operations
#include<iostream>
#include<climits>

// Prototype of a utility function to swap two integers
void swap(int *x, int *y);

// A class for Min Heap
struct MinHeap
{
	int *harr; // pointer to array of elements in heap
	int capacity; // maximum possible size of min heap
	int heap_size; // Current number of elements in min heap
};

// make a priority struct such that smaller priority is higher
struct priority
{
    int priority;
    int pid;
};

// Inserts a new key 'k'
void insertKey(struct MinHeap *minheap, int k)
{
	if (minheap->heap_size == minheap->capacity)
	{
		return;
	}

	// First insert the new key at the end
	minheap->heap_size++;
	int i = minheap->heap_size - 1;
	minheap->harr[i] = k;

	// Fix the min heap property if it is violated
	while (i != 0 && minheap->harr[parent(i)] > minheap->harr[i])
	{
        swap(&minheap->harr[i], &minheap->harr[parent(i)]);
        i = parent(i);
	}
}

// Decreases value of key at index 'i' to new_val. It is assumed that
// new_val is smaller than minheap->harr[i].
void decreaseKey(struct MinHeap *minheap, int i, int new_val)
{
	minheap->harr[i] = new_val;
	while (i != 0 && minheap->harr[parent(i)] > minheap->harr[i])
	{
	swap(&minheap->harr[i], &minheap->harr[parent(i)]);
	i = parent(i);
	}
}

// Method to remove minimum element (or root) from min heap
int extractMin(struct MinHeap *minheap)
{
	if (minheap->heap_size <= 0)
		return INT_MAX;
	if (minheap->heap_size == 1)
	{
		minheap->heap_size--;
		return minheap->harr[0];
	}

	// Store the minimum value, and remove it from heap
	int root = minheap->harr[0];
	minheap->harr[0] = minheap->harr[minheap->heap_size-1];
	minheap->heap_size--;
	MinHeapify(minheap, 0);

	return root;
}


// This function deletes key at index i. It first reduced value to minus
// infinite, then calls extractMin()
void deleteKey(struct MinHeap *minheap, int i)
{
	decreaseKey(minheap, i, INT_MIN);
	extractMin(minheap);
}

// A recursive method to heapify a subtree with the root at given index
// This method assumes that the subtrees are already heapified
void MinHeapify(struct MinHeap *minheap, int i)
{
	int l = left(i);
	int r = right(i);
	int smallest = i;
	if (l < minheap->heap_size && minheap->harr[l] < minheap->harr[i])
		smallest = l;
	if (r < minheap->heap_size && minheap->harr[r] < minheap->harr[smallest])
		smallest = r;
	if (smallest != i)
	{
		swap(&minheap->harr[i], &minheap->harr[smallest]);
		MinHeapify(minheap, smallest);
	}
}

// A utility function to swap two elements
void swap(int *x, int *y)
{
	int temp = *x;
	*x = *y;
	*y = temp;
}

// Driver program to test above functions
int main()
{
	MinHeap h(11);
	h.insertKey(3);
	h.insertKey(2);
	h.deleteKey(1);
	h.insertKey(15);
	h.insertKey(5);
	h.insertKey(4);
	h.insertKey(45);
	cout << h.extractMin() << " ";
	cout << h.getMin() << " ";
	h.decreaseKey(2, 1);
	cout << h.getMin();
	return 0;
}
