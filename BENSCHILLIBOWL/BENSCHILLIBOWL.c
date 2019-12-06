#include "BENSCHILLIBOWL.h"

#include <assert.h>
#include <stdlib.h>
#include <time.h>

void AddOrderToBack(Order **orders, Order *order);

MenuItem BENSCHILLIBOWLMenu[] = { 
    "BensChilli", 
    "BensHalfSmoke", 
    "BensHotDog", 
    "BensChilliCheeseFries", 
    "BensShake",
    "BensHotCakes",
    "BensCake",
    "BensHamburger",
    "BensVeggieBurger",
    "BensOnionRings",
};
int BENSCHILLIBOWLMenuLength = 10;

/* Select a random item from the Menu and return it */
MenuItem PickRandomMenuItem() {
    return BENSCHILLIBOWLMenu[rand() % BENSCHILLIBOWLMenuLength];
}

/* Allocate memory for the Restaurant, then create the mutex and condition variables needed to instantiate the Restaurant */
BENSCHILLIBOWL* OpenRestaurant(int max_size, int expected_num_orders) {
    BENSCHILLIBOWL *bcb = (BENSCHILLIBOWL*) malloc(sizeof(BENSCHILLIBOWL));
    bcb->orders = NULL;
    bcb->current_size = 0;
	  bcb->next_order_number=1;
    bcb->orders_handled=0;
    bcb->max_size = max_size;
    bcb->expected_num_orders=expected_num_orders;
  
    // using the mutex
    pthread_mutex_init(&(bcb->mutex), NULL);  
    printf("Restaurant is open!\n");
    return bcb;
}

/* check that the number of orders received is equal to the number handled (ie.fullfilled). Remember to deallocate your resources */
void CloseRestaurant(BENSCHILLIBOWL* bcb) {
    if (bcb->orders_handled != bcb->expected_num_orders) {  
        fprintf(stderr, "All orders were not fullfilled.\n");
        exit(0);
    }
    // destroy the synchronization variables
    pthread_mutex_destroy(&(bcb->mutex));  
    
    free(bcb);   
    printf("Restaurant is closed!\n");
}

/* add an order to the back of queue */
int AddOrder(BENSCHILLIBOWL* bcb, Order* order) {
    pthread_mutex_lock(&(bcb->mutex)); //grab the lock as order queue is in critical section
  
    // wait until restaurant is not full
    while (bcb->current_size == bcb->max_size) { 
        pthread_cond_wait(&(bcb->can_add_orders), &(bcb->mutex));
    }
  
    order->order_number = bcb->next_order_number;
    AddOrderToBack(&(bcb->orders), order);
  
    bcb->next_order_number++; 
    bcb->current_size++;   
    
    // send signal that an order had been placed
    pthread_cond_broadcast(&(bcb->can_get_orders)); 
    
    //release the lock 
    pthread_mutex_unlock(&(bcb->mutex)); 
    
    return order->order_number;
}

/* get an order from the front of the queue */
Order *GetOrder(BENSCHILLIBOWL* bcb) {
  
    // grab the lock as order is in critical section
    pthread_mutex_lock(&(bcb->mutex)); 
    
    while(bcb->current_size == 0) {  
        // release the mutex if all orders are fulfilled
        if (bcb->orders_handled >= bcb->expected_num_orders) {
            pthread_mutex_unlock(&(bcb->mutex));
            return NULL;
        }
        pthread_cond_wait(&(bcb->can_get_orders), &(bcb->mutex));
    }
    // Get the next order
    Order *order = bcb->orders;
    bcb->orders = bcb->orders->next;
    
    bcb->current_size--; 
    bcb->orders_handled++; 
    
    pthread_cond_broadcast(&(bcb->can_add_orders));
        
    //release the lock.
    pthread_mutex_unlock(&(bcb->mutex));   
    return order;
}

/* this method adds order to rear of queue */
void AddOrderToBack(Order **orders, Order *order) {
  if (*orders == NULL) { // no orders before
        *orders = order;
    } else {  //if there are some orders
        Order *curr_order = *orders;
        while (curr_order->next) {
            curr_order = curr_order->next;
        }
        curr_order->next = order;
    }
}