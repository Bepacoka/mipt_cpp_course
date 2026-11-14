#include "event_list.h"

namespace nano_edr {

void EventList::PushBack(const Event& event) {
    auto* node = new EventNode(event);

    if (tail_) {
        tail_->next = node;
        tail_ = node;
    } else {
        head_ = tail_ = node;
    }

    ++size_;
    if (capacity_ && size_ > capacity_) PopFront();
}

void EventList::PopFront() {
    if (!head_) return;

    auto* old_head = head_;
    head_ = head_->next;
    if (!head_) tail_ = nullptr;

    delete old_head;
    --size_;
}

void EventList::Clear() {
    while (head_) PopFront();
}

EventList::~EventList() {
    Clear();
}

}  // namespace nano_edr
