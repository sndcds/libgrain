//
//  List.hpp
//
//  Created by Roald Christesen on 27.03.2016
//  Copyright (C) 2025 Roald Christesen. All rights reserved.
//
//  This file is part of GrainLib, see <https://grain.one>.
//
//  LastChecked: 11.06.2025
//

//  https://www.internalpointers.com/post/writing-custom-iterators-modern-cpp

#ifndef GrainList_hpp
#define GrainList_hpp


#include "Grain.hpp"
#include "Type/Object.hpp"

#include <iterator>


namespace Grain {

    template <typename T>
    class List : public Object {
    public:
        static constexpr int64_t kMinStepSize = 16;

    protected:
        struct Iterator {
            using iterator_category = std::forward_iterator_tag;
            using difference_type = std::ptrdiff_t;
            using value_type = T;
            using pointer = T*;
            using reference = T&;

            explicit Iterator(pointer ptr) : ptr_(ptr) {}

            reference operator * () const { return *ptr_; }
            pointer operator -> () { return ptr_; }
            Iterator& operator ++ () { ptr_++; return *this; }
            Iterator operator ++ (int) { Iterator tmp = *this; ++(*this); return tmp; }
            Iterator& operator -- () { ptr_--; return *this; }
            Iterator operator -- (int) { Iterator tmp = *this; --(*this); return tmp; }
            Iterator operator + (difference_type n) const { return Iterator(ptr_ + n); }
            Iterator operator - (difference_type n) const { return Iterator(ptr_ - n); }
            difference_type operator - (const Iterator& other) const { return ptr_ - other.ptr_; }

            Iterator& operator += (difference_type n) { ptr_ += n; return *this; }
            Iterator& operator -= (difference_type n) { ptr_ -= n; return *this; }

            reference operator [] (difference_type n) const { return ptr_[n]; }

            friend bool operator == (const Iterator& a, const Iterator& b) { return a.ptr_ == b.ptr_; }
            friend bool operator != (const Iterator& a, const Iterator& b) { return a.ptr_ != b.ptr_; }
            friend bool operator < (const Iterator& a, const Iterator& b) { return a.ptr_ < b.ptr_; }
            friend bool operator > (const Iterator& a, const Iterator& b) { return a.ptr_ > b.ptr_; }
            friend bool operator <= (const Iterator& a, const Iterator& b) { return a.ptr <= b.ptr_; }
            friend bool operator >= (const Iterator& a, const Iterator& b) { return a.ptr_ >= b.ptr_; }

        protected:
            pointer ptr_;
        };

    protected:
        int64_t capacity_ = 0;                 ///< Maximum number of entries before reallocation is needed
        int64_t size_ = 0;                     ///< Current number of entries in the list
        int64_t grow_step_ = kMinStepSize;     ///< Number of entries to grow by when reallocating (if not doubling)
        bool double_capacity_mode_ = true;     ///< If `true`, the list doubles in size when reallocating memory

        T* data_ = nullptr;                    ///< Pointer to the allocated memory holding the elements
        T dummy_{};                            ///< Returned when an invalid index is accessed.
        size_t element_size_ = 0;              ///< Size of each element (optional if using `sizeof(T)` directly)
        SortCompareFunc sort_compare_func_ = nullptr;  ///< Custom comparison function used by sort()

    public:
        List() {
            element_size_ = sizeof(T);
            reserve(kMinStepSize);
        }

        explicit List(int64_t capacity) {
            element_size_ = sizeof(T);
            reserve(capacity);
        }

        ~List() override {
            std::free(data_);
        }

        [[nodiscard]] const char* className() const noexcept override {
            return "List";
        }

        friend std::ostream& operator << (std::ostream& os, const List* o) {
            return o == nullptr ? os << "List nullptr" : os << *o;
        }

        friend std::ostream& operator << (std::ostream& os, const List& o) {
            os << o.className() << " with " << o.size_ << " of " << o.capacity_ << " entries, entry size: " << o.element_size_ << " bytes";
            if (o.data_) {
                os << ", memory is allocated";
            }
            else {
                os << ", memory is not(!) allocated";
            }

            if (o.double_capacity_mode_) {
                os << ", grows with double size";
            }
            else {
                os << ", grows by " << o.grow_step_ << " entries";
            }

            return os;
        }


        // Non-const version of operator[]
        T& operator[](int64_t index) noexcept {
            if (index >= 0 && index < size_) {
                return data_[index];
            }
            else {
                return dummy_;
            }
        }

        // Const version of operator[]
        const T& operator[](int64_t index) const noexcept {
            if (index >= 0 && index < size_) {
                return data_[index];
            }
            else {
                return dummy_;
            }
        }

        void free() noexcept {
            if (data_) {
                std::free(data_);
                data_ = nullptr;
            }
            size_ = 0;
            capacity_ = 0;
        }

        [[nodiscard]] const T* dataPtr() const noexcept { return (T* )data_; }
        [[nodiscard]] T* mutDataPtr() const noexcept { return (T* )data_; }

        [[nodiscard]] int64_t capacity() const noexcept { return capacity_; }
        [[nodiscard]] int64_t growStep() const noexcept { return grow_step_; }
        [[nodiscard]] int64_t size() const noexcept { return size_; }
        [[nodiscard]] bool isEmpty() const noexcept { return size_ == 0; }
        [[nodiscard]] int64_t elementSize() const noexcept { return element_size_; }
        [[nodiscard]] int64_t memSize() const noexcept { return element_size_ * capacity_; }

        [[nodiscard]] bool hasIndex(int64_t index) const noexcept {
            return (index >= 0 && index < size_ && data_);
        }

        [[nodiscard]] int64_t lastIndex() const noexcept {
            return size_ - 1;
        }

        [[nodiscard]] int64_t nextCapacity() const noexcept {
            if (double_capacity_mode_) {
                return capacity_ * 2;
            }
            else {
                return capacity_ + grow_step_;
            }
        }

        bool reserve(int64_t capacity) noexcept {
            if (capacity == capacity_) {
                return true;
            }

            if (capacity < 0) {
                capacity = 0;
            }

            if (capacity < size_) {
                capacity = size_;
            }

            if (capacity == 0 && data_) {
                // Release memory when capacity becomes 0
                std::free(data_);
                data_ = nullptr;
                capacity_ = 0;
                return true;
            }

            if (capacity_ == 0 && capacity > 0) {
                // Allocate new memory
                data_ = (T* )std::malloc(sizeof(T) * capacity);
                if (!data_) {
                    return false;
                }
                capacity_ = capacity;
            }
            else {
                // Reallocate memory
                T* new_data = (T* )std::realloc(data_, sizeof(T) * capacity);
                if (!new_data) {
                    return false;
                }
                data_ = new_data;
                capacity_ = capacity;
            }

            return true;
        }

        void setGrowStep(int64_t step) noexcept {
            grow_step_ = step < kMinStepSize ? kMinStepSize : step;
        }

        void setDoubleCapacityMode(bool mode) noexcept {
            double_capacity_mode_ = mode;
        }

        virtual bool resize(int64_t new_size, T value) noexcept {
            if (new_size > size()) {
                int64_t new_n = new_size - size();
                if (!reserve(new_size)) {
                    return false;
                }
                for (int64_t i = 0; i < new_n; i++) {
                    push(value);
                }
            }
            else if (new_size < size()) {
                size_ = new_size;
            }
            return true;
        }

        bool shrink(int64_t extra_capacity = 0) noexcept {
            return reserve(size_ + extra_capacity);
        }

        virtual bool push(const T* element_ptr) noexcept {
            if (!element_ptr) {
                return false;
            }

            if (size_ >= capacity_) {
                bool result = reserve(nextCapacity());
                if (!result) {
                    return false;
                }
            }

            data_[size_] = *element_ptr;
            size_++;

            return true;
        }

        virtual bool push(const T element) noexcept {
            if (size_ >= capacity_) {
                bool result = reserve(nextCapacity());
                if (!result) {
                    return false;
                }
            }

            data_[size_] = element;
            size_++;

            return true;
        }

        bool pop(T* out_element) noexcept {
            if (size_ > 0 && out_element) {
                *out_element = data_[lastIndex()];
                removeLast();
                return true;
            }
            else {
                return false;
            }
        }

        [[nodiscard]] T first() noexcept { return elementAtIndex(0); }
        [[nodiscard]] T last() noexcept { return lastElement(); }


        virtual bool replaceElementAtIndex(int64_t index, const T* element_ptr) noexcept {
            if (!element_ptr || !hasIndex(index)) {
                return false;
            }
            else {
                data_[index] = *element_ptr;
                return true;
            }
        }

        virtual bool replaceLastElement(const T* element_ptr) noexcept {
            if (!element_ptr || size_ < 1) {
                return false;
            }
            else {
                data_[size_] = *element_ptr;
                return true;
            }
        }

        virtual void clear() noexcept {
            size_ = 0;
        }

        void setSortCompareFunc(SortCompareFunc func) noexcept {
            sort_compare_func_ = func;
        }

        virtual ErrorCode sort() noexcept {
            return sort(sort_compare_func_);
        }

        ErrorCode sort(SortCompareFunc func) noexcept {
            if (!data_ || size_ <= 1) return ErrorCode::None;
            if (!func) return ErrorCode::SortFailed; // comparator must be valid
            try {
                std::qsort(data_, size_, sizeof(T), func);
            }
            catch (...) {
                std::cout << "What" << std::endl;
            }
            return ErrorCode::None;
        }

        [[nodiscard]] int64_t indexForElement(const T element) const noexcept {
            for (int64_t i = 0; i < size_; i++) {
                if ( data_[i] == element) {
                    return i;
                }
            }
            return -1;
        }

        bool elementAtIndex(int64_t index, T& out_element) const noexcept {
            if (index >= 0 && index < size_) {
                out_element = data_[index];
                return true;
            }
            else {
                return false;
            }
        }

        [[nodiscard]] T elementAtIndex(int64_t index) const noexcept {
            if (index >= 0 && index < size_) {
                return data_[index];
            }
            else {
                return dummy_;
            }
        }

        [[nodiscard]] const T* elementPtrAtIndex(int64_t index) const noexcept {
            if (index >= 0 && index < size_) {
                return &data_[index];
            }
            else {
                return nullptr;
            }
        }

        [[nodiscard]] T* mutElementPtrAtIndex(int64_t index) const noexcept {
            if (index >= 0 && index < size_) {
                return &data_[index];
            }
            else {
                return nullptr;
            }
        }

        [[nodiscard]] T lastElement() const noexcept {
            if (size_ > 0) {
                return data_[size_ - 1];
            }
            else {
                return dummy_;
            }
        }

        [[nodiscard]] const T* lastElementPtr() const noexcept {
            if (size_ > 0) {
                return &data_[size_ - 1];
            }
            else {
                return nullptr;
            }
        }

        [[nodiscard]] T* mutLastElementPtr() const noexcept {
            if (size_ > 0) {
                return &data_[size_ - 1];
            }
            else {
                return nullptr;
            }
        }

        bool swapElements(int64_t index_a, int64_t index_b) noexcept {
            if (index_a >= 0 && index_a < size_ && index_b >= 0 && index_b < size_) {
                T temp = data_[index_a];
                data_[index_a] = data_[index_b];
                data_[index_b] = temp;
                return true;
            }
            else {
                return false;
            }
        }

        /**
         *  @brief Remove element at a given index.
         *
         *  @param index The index where to remove the reference.
         *  @return `ErrorCode::None` if successful, or an error code if an
         *          error occurred.
         */
        virtual ErrorCode removeAtIndex(int64_t index) noexcept {
            std::cout << "removeAtIndex: " << index << std::endl;
            if (!hasIndex(index)) {
                std::cout << "IndexOutOfRange!" << std::endl;
                return ErrorCode::IndexOutOfRange;
            }

            size_--;
            std::cout << "new size_: " << size_ << std::endl;
            // Reorganize data if necessary
            if (index < size_) {
                for (int64_t i = index; i < size_; i++) {
                    data_[i] = data_[i + 1];
                }
            }

            std::cout << "reorganized OK\n";
            return ErrorCode::None;
        }

        /**
         *  @brief Remove element at a given index with possible reordering of
         *         elements.
         *
         *  @param index The index where to remove the reference.
         *  @return ErrorCode::None on success, or an appropriate ErrorCode on
         *          failure.
         */
        virtual ErrorCode removeAtIndexReorderingAllowed(int64_t index) noexcept {
            if (!hasIndex(index)) {
                return ErrorCode::IndexOutOfRange;
            }

            int64_t last_index = size_ - 1;
            if (index != last_index) {
                swapElements(index, last_index);
            }

            size_--;
            return ErrorCode::None;
        }

        ErrorCode removeElement(const T element) noexcept {
            for (int64_t i = 0; i < size_; i++) {
                if (data_[i] == element) {
                    return removeAtIndex(i);
                }
            }
            return ErrorCode::NoMatch;
        }

        virtual ErrorCode removeLast() noexcept {
            if (size_ > 0) {
                return removeAtIndex(lastIndex());
            }
            else {
                return ErrorCode::None;
            }
        }

        Iterator begin() const {
            return Iterator(&data_[0]);
        }

        Iterator end() const {
            return Iterator(&data_[size_]);
        }
    };


/**
 *  @class ObjectList
 *  @brief A specialized list for managing objects with automatic memory
 *         management.
 *
 *  The `ObjectList` class is a derived class from `List<T>` that provides
 *  additional functionality for managing objects of type `T`. It ensures that
 *  objects are automatically retained when added to the list and released when
 *  removed. This class is primarily designed for managing dynamic collections
 *  of objects, offering methods for insertion, removal, and list management.
 *
 *  The `ObjectList` class also overrides the `clear` and `push` methods to
 *  properly manage memory for the contained objects.
 *
 *  @tparam T The type of the objects stored in the list, typically pointers to
 *            objects.
 */
    template <typename T>
    class ObjectList : public List<T> {

    public:
        /**
         *  @brief Default constructor for `ObjectList`.
         *
         *  This constructor initializes the `ObjectList` with a default capacity
         *  of `kMinStepSize`.
         */
        ObjectList() : List<T>(List<T>::kMinStepSize) {
        }

        /**
         *  @brief Constructor for `ObjectList` with a specified capacity.
         *
         *  This constructor initializes the `ObjectList` with the specified
         *  capacity.
         *
         *  @param capacity The initial capacity of the list.
         */
        ObjectList(int64_t capacity) : List<T>(capacity) {
        }

        /**
         *  @brief Destructor for `ObjectList`.
         *
         *  The destructor clears the list and ensures all objects are released
         *  properly.
         */
        virtual ~ObjectList() {
            clear();
        }

        const char* className() const noexcept override { return "ObjectList"; }


        /**
         *  @brief Clears all elements in the list and releases the objects.
         *
         *  This method overrides the `clear` method and ensures all objects in the
         *  list are released
         *  using the `GRAIN_RELEASE` macro. It then calls the `clear` method of the
         *  base `List` class to remove all elements.
         */
        virtual void clear() noexcept override {
            auto p = (Object** )List<T>::mutDataPtr();
            for (int64_t i = 0; i < List<T>::size_; i++) {
                GRAIN_RELEASE(p[i]);
            }
            List<T>::clear();
        }

        /**
         *  @brief Adds an object to the list with modifying ownership.
         *
         *  This method allows inserting an object into the list without altering
         *  its reference count. The `ObjectList` becomes the new owner of `ob`.
         *
         *  @param ob The object to add to the list.
         *  @return `true` if the object was successfully added, `false` if the
         *          object is `nullptr`.
         */
        virtual bool push(T ob) noexcept override {
            if (!ob) {
                return false;
            }
            bool result = List<T>::push(ob);
            return result;
        }

        /**
         *  @brief Inserts an object at a specific index in the list with modifying
         *         ownership.
         *
         *  This method inserts an object at the given index, shifting the existing
         *  elements to the right. If necessary, it will automatically grow the list
         *  to accommodate the new element. The `ObjectList` becomes the new owner
         *  of `ob`.
         *
         *  @param index The index at which to insert the object.
         *  @param ob The object to insert into the list.
         *  @return An `ErrorCode` representing the result of the insertion.
         *    - `ErrorCode::None` if insertion succeeded.
         *    - `ErrorCode::BadArgs` if the arguments are invalid.
         *    - `ErrorCode::MemCantGrow` if memory growth failed.
         */
        ErrorCode insertAtIndexChangeOwner(int64_t index, T ob) noexcept {
            if (!ob) {
                return ErrorCode::BadArgs;
            }
            if (index < 0 || index > this->size_) {
                return ErrorCode::BadArgs;
            }
            if (this->size_ >= this->m_capacity) {
                auto flag = List<T>::reserve(this->m_capacity + this->m_grow_step);
                if (!flag) {
                    return ErrorCode::MemCantGrow;
                }
            }

            auto data = List<T>::mutDataPtr();

            // Move all references behind index position to the right
            for (int64_t i = this->size_; i > index; i--) {
                data[i] = data[i - 1];
            }

            // Insert the new reference
            data[index] = ob;
            this->size_++;

            return ErrorCode::None;
        }

        /**
         *  @brief Removes the object at the specified index from the list.
         *
         *  This method removes the object at the specified index and releases it
         *  using the `GRAIN_RELEASE` macro.
         *
         *  @param index The index of the object to remove.
         *  @return An `ErrorCode` representing the result of the removal.
         *    - `ErrorCode::None` if the removal succeeded.
         *    - Any other error code if removal failed.
         */
        ErrorCode removeAtIndex(int64_t index) noexcept override {
            auto ob = List<T>::elementAtIndex(index);
            auto result = List<T>::removeAtIndex(index);
            if (result == ErrorCode::None) {
                auto _ob = (Object*)ob;
                GRAIN_RELEASE(_ob);
            }
            return result;
        }
    };


} // End of namespace Grain

#endif // GrainList_hpp
