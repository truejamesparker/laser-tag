/*
 * queue.c
 *
 *  Created on: Jan 6, 2015
 *      Author: jwparker
 */

#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

// Standard queue implementation that leaves one spot empty so easier 
// to check for full/empty.
void queue_init(queue_t* q, queue_size_t size) {
  q->indexIn = 0;
  q->indexOut = 0;
  q->elementCount = 0;
  q->size = size;	// Add one additional location for the empty location.
  q->data = (queue_data_t *) malloc(size * sizeof(queue_data_t));
}

// Returns the size of the queue..
queue_size_t queue_size(queue_t* q) {
	return q->size;
}

// Returns true if the queue is full.
bool queueFull(queue_t* q) {
	return (q->elementCount==q->size);
}

// Returns true if the queue is empty.
bool queue_empty(queue_t* q) {
	return (q->elementCount==0);
}

// Pushes a new element into the queue. Reports an error if the queue is full.
void queue_push(queue_t* q, queue_data_t value) {
	if (queueFull(q)){
		printf("Error! Queue is full!\n");
	}
	else {
		q->data[q->indexIn] = value;
		q->indexIn++;
		// Check to see if we are at the end of the queue
		if (q->indexIn == q->size) {
			q->indexIn = 0;
		}
		q->elementCount++;
	}
}

// Removes the oldest element in the queue.
queue_data_t queue_pop(queue_t* q) {

	if (q->elementCount==0) {
		printf("Attempting to pop from an empty queue!\n");
		return 0;
	}

	queue_data_t _data = q->data[q->indexOut];
	q->indexOut++;
	q->elementCount--;
	// Check to see if we are at the end of the queue
	if (q->indexOut == q->size) {
		q->indexOut = 0;
	}
	return _data;
}

// Pushes a new element into the queue, making room by removing the oldest element.
void queue_overwritePush(queue_t* q, queue_data_t value) {
	if (queueFull(q)) {
		queue_pop(q);
	}
	queue_push(q, value);
}

// Provides random-access read capability to the queue.
// Low-valued indexes access older queue elements 
// while higher-value indexes access newer elements
// (according to the order that they were added).
queue_data_t queue_readElementAt(queue_t* q, queue_index_t index) {
	queue_index_t _index;
	_index = q->indexOut;
	return (q->data)[(_index + index) % (q->size)];
}

// Returns a count of the elements currently contained in the queue.
queue_size_t queue_elementCount(queue_t* q) {
	return q->elementCount;
}

// Prints the current contents of the queue. Handy for debugging.
void queue_print(queue_t* q) {
	for (int i=0; i<(int)q->elementCount; i++) {
		printf("%f\n", queue_readElementAt(q, i));
	}
}

// Just free the malloc'd storage.
void queue_garbageCollect(queue_t* q) {
  free(q->data);
}

// Empties the queue by repeatedly calling queue_pop()
void queue_clear(queue_t* q) {
	queue_size_t elements = q->elementCount;
	for (int i=0; i<(int)elements; i++) {
		queue_pop(q);
	}
}

/*// Performs a comprehensive test of all queue functions.
int queue_runTest() {
	printf("Beginning test!\n");
	queue_t queue;
	queue_size_t size = 10;
	queue_init(&queue, size);
	//queue_runTest();

	printf("Size of Queue: %lu\n", queue_size(&queue));
	if (size!=queue_size(&queue)){
		printf("Error! Queue was initialized in wrong size!\n");
	}

	////////////// PUSHING //////////////////
	
	printf("Testing pushing capability\n");
	int toPush = rand() % size;
	for (int i=0; i<toPush; i++) {
		queue_push(&queue, rand());
	}

	printf("Pushed %u elements to the queue\n", toPush);
	printf("Number of elements %lu\n", queue_elementCount(&queue));

	queue_clear(&queue);

	printf("Attempting to push to full queue\n");
	printf("Verify error statement below:\n");
	for (int i=0; i<size+1; i++) {
		queue_push(&queue, rand());
	}

	printf("Using overwritePush on full queue\n");
	queue_clear(&queue);
	for (int i=0; i<size; i++) {
			queue_push(&queue, i+1);
		}
	printf("Current queue:\n\n");
	queue_print(&queue);
	queue_data_t oldData = queue_readElementAt(&queue, 0);
	queue_data_t newData = rand();
	queue_overwritePush(&queue, newData);
	printf("\nNew queue:\n\n");
	queue_print(&queue);

	if (queue_readElementAt(&queue, size-1)!=newData) {
		printf("Failed to perform overwritePush\n");
	}
	else {
		printf("\nFunction overwritePush successful!\n");
	}

	printf("Function readElementAt successful!\n");

	/////////////// PRINTING /////////////////
	
	printf("Verifying queueFull functionality\n");
	if(queueFull(&queue))
		printf("Queue is full\n");
	else {
		printf("Queue is not full\n");
	}
	printf("Clearing queue\n");
	queue_clear(&queue);

	printf("Verifying queue_empty functionality\n");
	if(queue_empty(&queue))
			printf("Queue is empty\n");
		else {
			printf("Queue is not empty\n");
		}

	/////////////// POPPING ///////////////////

	printf("Attempting to pop from an empty queue\n");
	printf("Verify error statement below:\n");
	queue_pop(&queue);



	printf("Executing garbageCollect\n");
	queue_garbageCollect(&queue);
}*/


#define SMALL_QUEUE_SIZE 1000
#define SMALL_QUEUE_COUNT 10
static queue_t smallQueue[SMALL_QUEUE_COUNT];
static queue_t largeQueue;

// smallQueue[SMALL_QUEUE_COUNT-1] contains newest value, smallQueue[0] contains oldest value.
// Thus smallQueue[0](0) contains oldest value. smallQueue[SMALL_QUEUE_COUNT-1](SMALL_QUEUE_SIZE-1) contains newest value.
// Presumes all queue come initialized full of something (probably zeros).
static double popAndPushFromChainOfSmallQueues(double input) {
	// Grab the oldest value from the oldest small queue before it is "pushed" off.
	double willBePoppedValue = queue_readElementAt(&(smallQueue[0]), 0);
	// Sequentially pop from the next newest queue and push into next oldest queue.
	for (int i=0; i<SMALL_QUEUE_COUNT-1; i++) {
		queue_overwritePush(&(smallQueue[i]), queue_pop(&(smallQueue[i+1])));
	}
	queue_overwritePush(&(smallQueue[SMALL_QUEUE_COUNT-1]), input);
	return willBePoppedValue;
}

static bool compareChainOfSmallQueuesWithLargeQueue() {
	bool success = true;
	// Start comparing the oldest element in the chain of small queues, and the large queue
	// and move towards the newest values.
	for (uint16_t smallQIdx=0; smallQIdx<SMALL_QUEUE_COUNT; smallQIdx++) {
    for (uint16_t smallQEltIdx=0; smallQEltIdx<SMALL_QUEUE_SIZE; smallQEltIdx++) {
    	double smallQElt = queue_readElementAt(&(smallQueue[smallQIdx]), smallQEltIdx);
    	double largeQElt = queue_readElementAt(&largeQueue, (smallQIdx*SMALL_QUEUE_SIZE) + smallQEltIdx);
    	if (smallQElt != largeQElt) {
    		printf("not-equal\n\r");
    		printf("largeQ(%d):%lf\n\r", (smallQIdx*SMALL_QUEUE_SIZE) + smallQEltIdx, largeQElt);
    		printf("smallQ[%d](%d): %lf\n\r", smallQIdx, smallQEltIdx, smallQElt);
        success = false;
        break;
      }
    }
	}
	return success;
}

#define TEST_ITERATION_COUNT 10000
bool queue_runTest() {
	bool success = true;  // Be optimistic.
	// Let's make this a real torture test by testing queues against themselves.
	// Test the queue against an array to make sure there is agreement between the two.
  double testData[SMALL_QUEUE_SIZE];
  queue_t q;
  queue_init(&q, SMALL_QUEUE_SIZE);
  // Generate test values and place the values in both the array and the queue.
  for (int i=0; i<SMALL_QUEUE_SIZE; i++) {
  	double value = (double)rand()/(double)RAND_MAX;
  	queue_overwritePush(&q, value);
  	testData[i] = value;
  }
  // Everything is initialized, compare the contents of the queue against the array.
  for (int i=0; i<SMALL_QUEUE_SIZE; i++) {
  	double qValue = queue_readElementAt(&q, i);
  	if (qValue != testData[i]) {
  		printf("testData[%d]:%lf != queue_readElementAt(&q, %d):%lf\n\r", i, testData[i], i, qValue);
  		success = false;
  	}
  }
  if (!success) {
  	printf("Test 1 failed. Array contents not equal to queue contents.\n\r");
  } else {
  	printf("Test 1 passed. Array contents match queue contents.\n\r");
  }
  success = true;  // Remain optimistic.
	// Test 2: test a chain of 5 queues against a single large queue that is the same size as the cumulative 5 queues.
	for (int i=0; i<SMALL_QUEUE_COUNT; i++)
		queue_init(&(smallQueue[i]), SMALL_QUEUE_SIZE);
	for (int i=0; i<SMALL_QUEUE_COUNT; i++) {
		for (int j=0; j<SMALL_QUEUE_SIZE; j++)
			queue_overwritePush(&(smallQueue[i]), 0.0);
	}
	queue_init(&largeQueue, SMALL_QUEUE_SIZE * SMALL_QUEUE_COUNT);
  for (int i=0; i<SMALL_QUEUE_SIZE*SMALL_QUEUE_COUNT; i++)
		queue_overwritePush(&largeQueue, 0.0);
	for (int i=0; i<TEST_ITERATION_COUNT; i++) {
		double newInput = (double)rand()/(double)RAND_MAX;
		popAndPushFromChainOfSmallQueues(newInput);
		queue_overwritePush(&largeQueue, newInput);
		if (!compareChainOfSmallQueuesWithLargeQueue()) {
			success = false;
			break;
		}
	}

	if (success)
		printf("Test 2 passed.\n\r");
	else
		printf("Test 2 failed. The content of the chained small queues does not match the contents of the large queue.\n\r");
	return success;
}
