CSF Assignment 6
Christian Helgeson and Kevin Velasquez

Using a std::mutex within calc.cpp, it was ensured that the shared calculator data was not modified concurrently. Every time the unordered_map was about to be modified, the mutex would be locked, only to later be unlocked once the modification of variables was complete.
