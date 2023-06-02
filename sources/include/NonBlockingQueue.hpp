#pragma once
#include <atomic>
#include <vector>

template <typename T> class NonBlockingQueue {
    struct CellTy {
        std::atomic<unsigned> Sequence;
        T Data;
    };

    std::vector<CellTy> Buffer;
    unsigned BufferMask;
    std::atomic<unsigned> EnqueuePos, DequeuePos;

public:
    explicit NonBlockingQueue(unsigned BufSize) : Buffer(BufSize), BufferMask(BufSize - 1) {
        if (BufSize > (1 << 30))
            throw std::runtime_error("buffer size too large");

        if (BufSize < 2)
            throw std::runtime_error("buffer size too small");

        if ((BufSize & (BufSize - 1)) != 0)
            throw std::runtime_error("buffer size is not power of 2");

        for (unsigned i = 0; i != BufSize; ++i)
            Buffer[i].Sequence.store(i);

        EnqueuePos.store(0);
        DequeuePos.store(0);
    }
    NonBlockingQueue(NonBlockingQueue&& other) {
        *this = std::move(other);
    }

    NonBlockingQueue& operator= (NonBlockingQueue&& other) {
        if (this != &other) {
            this->Buffer = std::move(other.Buffer);
            this->BufferMask = other.BufferMask;
            this->EnqueuePos = other.EnqueuePos.load();
            this->DequeuePos = other.DequeuePos.load();
        }

        return *this;
    }

    bool push(T Data) {
        CellTy *Cell;
        unsigned Pos;
        bool Res = false;

        while (!Res) {
            Pos = EnqueuePos.load();
            Cell = &Buffer[Pos & BufferMask];
            auto Seq = Cell->Sequence.load();
            auto Diff = static_cast<int>(Seq) - static_cast<int>(Pos);

            if (Diff < 0)
                return false;

            if (Diff == 0)
                Res = EnqueuePos.compare_exchange_weak(Pos, Pos + 1);
        }

        Cell->Data = std::move(Data);
        Cell->Sequence.store(Pos + 1);
        return true;
    }

    bool pop(T &Data) {
        CellTy *Cell;
        unsigned Pos;
        bool Res = false;

        while (!Res) {
            Pos = DequeuePos.load();
            Cell = &Buffer[Pos & BufferMask];
            auto Seq = Cell->Sequence.load();
            auto Diff = static_cast<int>(Seq) - static_cast<int>(Pos + 1);

            if (Diff < 0)
                return false;

            if (Diff == 0)
                Res = DequeuePos.compare_exchange_weak(Pos, Pos + 1);
        }

        Data = std::move(Cell->Data);
        Cell->Sequence.store(Pos + BufferMask + 1);
        return true;
    }

    [[nodiscard]] bool empty() const { return EnqueuePos.load() == DequeuePos.load(); }
};
