#include <iostream>
#include <random>
#include <cstdlib>
#include <ctime>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

 
template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 

    std::unique_lock<std::mutex> msgLock(_mutex);
    _condition.wait(msgLock, [this](){return !_queue.empty();});
    T msg = std::move(_queue.back());
    _queue.pop_back();
    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> msgLock(_mutex);
    _queue.push_back(std::move(msg));
    _condition.notify_one();
}


/* Implementation of class "TrafficLight" */

 
TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
    _msgQueue = std::make_shared<MessageQueue<TrafficLightPhase>>();
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while(true){
        if (_msgQueue->receive() == TrafficLightPhase::green){return;}
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class.
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this)); 
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles.

    // store last_updated_time
    auto last_updated_time = std::chrono::high_resolution_clock::now();
    
    // generate random number for cycle duration
    std::srand(std::time(0));
    int random_cycle_duration = 4;//std::rand() % 3 + 4;
    while(true){        
        auto curr_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> time_passed = curr_time - last_updated_time;
        //std::cout << "Time passed = " << time_passed.count() << " Cycle duration = " << random_cycle_duration << "\n";
        if (time_passed.count() >= random_cycle_duration){
            // toggle phase            
            if (_currentPhase == TrafficLightPhase::red){
                _currentPhase = TrafficLightPhase::green;
            } else {
                _currentPhase = TrafficLightPhase::red;
            }

            // send message to message_queue
            _msgQueue->send(std::move(_currentPhase));

            // update the last_update_time
            last_updated_time = std::chrono::high_resolution_clock::now();
        }

        // sleep between two cycles
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    } 
}

