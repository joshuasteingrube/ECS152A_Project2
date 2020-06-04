#include <iostream>

struct event
{
    int time;
    int type;
    event* nxt;
    event* prev;
    event(int val, int val2) : time(val), type(val2), nxt(nullptr), prev(nullptr) {}
};

template<class T>
class GEL
{
    int size;

public:
    event* head, * tail;

    GEL() : head(nullptr), tail(nullptr), size(0) {}

    ~GEL()
    {
        event* tmp = nullptr;
        while (head)
        {
            tmp = head;
            head = head->nxt;
            delete tmp;
        }
        head = nullptr;
    }

    GEL(const GEL<T>& dll) = delete;
    GEL& operator=(GEL const&) = delete;

    //inserts the event dependng on what time it is
    void insert(event* new_event)
    {
        struct event* current_event = head;
        //struct eventNode* my_new_event(new_event->time, new_event->type);
        bool event_placed = false;

        if (size <= 0)
        {
            head = new_event;
            tail = new_event;
            new_event->nxt = nullptr;
            new_event->prev = nullptr;

        }
        else {
            //iterate through the gel and find where to place new event based on time
            while (!event_placed) {
                if (new_event->time < current_event->time) {
                    new_event->nxt = current_event;
                    new_event->prev = current_event->prev;
                    current_event->prev = new_event;
                    //check if the new event will be the the first or last event
                    if (current_event == head) {
                        head = new_event;
                    }
                    event_placed = true;
                }
                else {
                    //if new event is last event
                    if (current_event == tail) {
                        new_event->prev = current_event;
                        current_event->nxt = new_event;
                        tail = new_event;
                        event_placed = true;
                    }
                    current_event = current_event->nxt;
                }
            }
        }
        size = size + 1;
        std::cout << "inserted\n";
    }

    event* removeFirst(void)
    {
        event* first = head;
        if(size > 1){
            event* second = head->nxt;
            second->prev = nullptr;
            head = second;
        }
        first->nxt = nullptr;
        size = size - 1;
        if(size < 1){
            head = nullptr;
        }
        std::cout << "removed first.  size is now: " << size << "\n";
        return first;
        
    }

    template <class U>
    friend std::ostream& operator<<(std::ostream& os, const GEL<U>& dll) {
        dll.display(os);
        return os;
    }

private:

    void display(std::ostream& out = std::cout) const
    {
        event* node = head;
        while (node != nullptr)
        {
            out << node->time << " " << node->type << "\n";
            node = node->nxt;
        }
    }
};

/*int main(){
  std::cout<<"starting\n";

  GEL<event> *l1 = new GEL<event>();
  event* myevent = new event(2, 0); 
  event* myevent2 = new event(1, 0);

  l1->insert(myevent);
  std::cout<<"head time = "<<l1->head->time<<"\n";
  l1->insert(myevent2);
  std::cout<<"head time = "<<l1->head->time<<"\n";
  event* myevent3 = l1->removeFirst();
  std::cout<<"removed item = "<<myevent3->time<<"\n";
  std::cout<<"head time = "<<l1->head->time<<"\n";
  myevent3 = l1->removeFirst();
  std::cout<<"removed item = "<<myevent3->time<<"\n";
  //std::cout<<"head time = "<<l1->head->time<<"\n";
  return 0;
}*/
  