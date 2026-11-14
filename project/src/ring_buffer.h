// Заготовка кольцевого буфера. Занятие 3.1.
//
// Скопируйте файл в `src/` и доведите до рабочего состояния. С этого момента
// он ваш: правьте что угодно, включая этот комментарий.
//
// ЧТО ЗДЕСЬ ЛЕЖИТ
//
// Публичный интерфейс целиком и ни одного тела. Интерфейс менять нельзя —
// против него собраны выданные тесты и выданный `ThresholdRule`. Всё
// остальное ваше: приватные поля, как считается физический индекс, как
// устроен итератор.
//
// ЗАДАНИЕ
//
// 1. Сделать буфер шаблоном `RingBuffer<T, N>`. Сейчас он хранит `Timestamp`,
//    а вместимость задана константой — так его хватило бы одному
//    `ThresholdRule` и никому больше: окно агента хранит не отметки времени.
//    `T` — параметр-тип, `N` — non-type параметр (лекция 8): размер попадает
//    в тип, поэтому `RingBuffer<int, 4>` и `RingBuffer<int, 64>` — разные
//    типы, и перепутать их нельзя.
// 2. Написать реализацию. Тела пишутся прямо здесь: шаблон нельзя разложить
//    на заголовок и `.cpp` — компилятору нужно тело в той единице трансляции,
//    где шаблон используется.
//
// Требования к поведению и границы, которые проверяются, — в постановке
// занятия.

#ifndef NANO_EDR_RING_BUFFER_H
#define NANO_EDR_RING_BUFFER_H

#include <array>
#include <cstddef>

namespace nano_edr {

template <typename T, size_t N>
class RingBuffer {
 public:
    // Обход в логическом порядке — от самого старого к самому новому.
    //
    // Итератор минимальный: ровно то, что нужно range-based for. Как он
    // добирается до элементов буфера, решаете вы; приватной части здесь нет
    // намеренно.

    RingBuffer() = default;
    
    class Iterator {
      public:
        // я думаю конструктор итератора тоже не помешает
        Iterator(size_t index, size_t remaining, const T* arr) : index_(index), remaining_(remaining), arr_(arr) {}
        const T& operator*() const {
            return arr_[index_];
        }
        Iterator& operator++() {
            if (++index_ == N) index_ = 0;
            --remaining_;
            return *this;
        }
        bool operator!=(const Iterator& other) {
            return index_ != other.index_ || remaining_ != other.remaining_; // вероятно, хватит только remaining но мало ли
        }
      private:
        size_t index_ = 0;
        size_t remaining_ = 0;
        const T* arr_ = nullptr;
    };

    void PushBack(const T& value) {
        buf_[index_++] = value;
        if (index_ == N) {
            full_ = true; // мы можем несколько раз это сделать, но кого волнует
            index_ = 0;
        }
    }

    // вроде как полностью удаление не просят, так что удаление будет ленивым
    void Clear() {
        full_ = false;
        index_ = 0;
        buf_ = {};
    }

    size_t size() const {
        if (full_) return N;
        return index_;
    }
    
    static constexpr size_t capacity() { return N; }
    bool empty() const { return !full_ && !index_; }
    bool full() const { return full_; }

    const T& operator[](size_t index) const {
        if (!full_) return buf_[index];
        return buf_[(index_ + index) % N];
    }

    // Самый новый элемент. Вызывать на пустом буфере нельзя — это ошибка
    // в коде вызывающего, а не случай, который надо обрабатывать.
    const T& back() const {
        if (!full_) return buf_[index_ - 1]; // соответствует комментарию выше
        return buf_[(index_ + N - 1) % N];
    }

    Iterator begin() const {
        if (full_) return Iterator(index_, N, buf_.data());
        return Iterator(0, index_, buf_.data());
    }
    Iterator end() const {
        return Iterator(index_, 0, buf_.data());
    }
  private:
    size_t index_ = 0;
    bool full_ = false;
    std::array<T, N> buf_ = {};
};

}  // namespace nano_edr

#endif  // NANO_EDR_RING_BUFFER_H
