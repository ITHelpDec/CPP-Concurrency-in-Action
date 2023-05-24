class queue {
    // ...
    std::unique_ptr<node> pop_head()
    {
        // call get_tail() before lock
        node *old_tail = get_tail();
    
        // carry on as before
        std::lock_guard lock(head_m);
    
        // if (head_.get() == get_tail()) { return nullptr; }
        if (head_.get() == old_tail) { return nullptr; }
            std::unique_ptr<node> old_head = std::move(head_);
            head_ = std::move(old_head->next_);
            return old_head;
        }
        // ...
    }    
    // ...
};
