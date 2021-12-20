#pragma once

#include <map>

namespace util
{
    template <class T> class shared_ptr
    {
    public:
        template <class T, class P1> friend shared_ptr<T> make_shared(P1 const& p1);
        template <class T> friend class weak_ptr;

        shared_ptr()
            : data(0), controlBlock(0)
        {}

        shared_ptr(T* const data)
            : data(data)
        {
            typename std::map<T*, size_t*>::iterator it = controlBlockTable().find(data);
            if (it != controlBlockTable().end())
            {
                controlBlock = it->second;
                controlBlock[0];
            }
            else
            {
                controlBlock = new size_t[2];
                controlBlock[0] = 1;
                controlBlock[1] = 0;
            }

            controlBlockTable()[data] = controlBlock;
        }

        shared_ptr(shared_ptr const& other)
            : data(other.data), controlBlock(other.controlBlock)
        {
            if (controlBlock || data)
            {
                increaseRefCount();
            }
        }

        shared_ptr& operator = (shared_ptr const& other)
        {
            if (controlBlock || data)
            {
                decreaseRefCount();
            }

            data = other.data;
            controlBlock = other.controlBlock;

            if (controlBlock || data)
            {
                increaseRefCount();
            }

            return *this;
        }

        T*& operator = (T* const other)
        {
            if (controlBlock || data)
            {
                decreaseRefCount();
            }

            typename std::map<T*, size_t*>::iterator it = controlBlockTable().find(data);
            if (it != controlBlockTable().end())
            {
                controlBlock = it->second;
                ++controlBlock[0];
            }
            else
            {
                controlBlock = new size_t[2];
                controlBlock[0] = 1;
                controlBlock[1] = 0;
            }

            data = other.data;
            return data;
        }

        ~shared_ptr()
        {
            reset();
        }

        void reset()
        {
            if (!data && !controlBlock)
            {
                return;
            }

            if (!controlBlock)
            {
                size_t* tmpControlBlock = reinterpret_cast<size_t*>(data) - 2;
                if (!--tmpControlBlock[0] && tmpControlBlock[1] == 0)
                {
                    data->~T();
                    delete[] reinterpret_cast<char*>(tmpControlBlock);
                }
            }
            else
            {
                if (!--controlBlock[0])
                {
                    delete data;
                    controlBlockTable().erase(data);
                }
                if (controlBlock[0] == 0 && controlBlock[1] == 0)
                {
                    delete[] controlBlock;
                }
            }

            data = 0;
            controlBlock = 0;
        }

        T* get() const
        {
            return data;
        }

        T& operator*() const
        {
            return *data;
        }

        T* operator ->() const
        {
            return data;
        }

    private:
        size_t increaseRefCount()
        {
            return controlBlock ? ++controlBlock[0] : ++(reinterpret_cast<size_t*>(data) - 2)[0];
        }

        size_t decreaseRefCount()
        {
            return controlBlock ? --controlBlock[0] : --(reinterpret_cast<size_t*>(data) - 2)[0];
        }

        shared_ptr(T* const data, size_t* const controlBlock)
            : data(data), controlBlock(controlBlock)
        {
            if (controlBlock || data)
            {
                increaseRefCount();
            }
        }

        static std::map<T*, size_t*>& controlBlockTable()
        {
            static std::map<T*, size_t*> table;
            return table;
        }

        T* data;
        size_t* controlBlock;
    };

    template <class T, class P1> shared_ptr<T> make_shared(P1 const& p1) //would be overloaded
    {
        try
        {
            size_t* control = reinterpret_cast<size_t*>(new char[sizeof(size_t) * 2 + sizeof(T)]);
            new(control + 2)T(p1);
            control[0] = 0;
            control[1] = 0;

            return shared_ptr<T>(reinterpret_cast<T*>(control + 2), 0);
        }
        catch (...)
        {
            return shared_ptr<T>();
        }
    }

    template <class T> class weak_ptr
    {
    public:
        weak_ptr()
            : data(0), controlBlock(0)
        {}

        weak_ptr(weak_ptr<T> const& other)
            : data(other.data), controlBlock(other.controlBlock)
        {
            if (controlBlock || data)
            {
                increaseRefCount();
            }
        }

        weak_ptr(shared_ptr<T> const& other)
            : data(other.data), controlBlock(other.controlBlock)
        {
            if (controlBlock || data)
            {
                increaseRefCount();
            }
        }

        ~weak_ptr()
        {
            reset();
        }

        weak_ptr& operator = (weak_ptr const& other)
        {
            if (controlBlock || data)
            {
                decreaseRefCount();
            }

            data = other.data;
            controlBlock = other.controlBlock;

            if (controlBlock || data)
            {
                increaseRefCount();
            }

            return *this;
        }

        weak_ptr<T>& operator = (shared_ptr<T> const& other)
        {
            if (controlBlock || data)
            {
                decreaseRefCount();
            }

            data = other.data;
            controlBlock = other.controlBlock;

            if (controlBlock || data)
            {
                increaseRefCount();
            }

            return *this;
        }

        T* get() const
        {
            return data;
        }

        bool isAlive() const
        {
            return data && controlBlock && controlBlock[0] != 0 || data && !controlBlock && (reinterpret_cast<size_t*>(data) - 2)[0] != 0;
        }

        void reset()
        {
            if (data && controlBlock)
            {
                if (decreaseRefCount() == 0 && controlBlock[0] == 0)
                {
                    delete[] controlBlock;
                }
            }
            else if (data)
            {
                size_t* tmpControlBlock = reinterpret_cast<size_t*>(data) - 2;
                if (decreaseRefCount() == 0 && tmpControlBlock[0] == 0)
                {
                    delete[] reinterpret_cast<char*>(tmpControlBlock);
                }
            }

            data = 0;
            controlBlock = 0;
        }

        shared_ptr<T> lock()
        {
            if (!data && !controlBlock)
            {
                return shared_ptr<T>();
            }
            else if (data && controlBlock)
            {
                if (controlBlock[0] != 0)
                {
                    return shared_ptr<T>(data, controlBlock);
                }
            }
            else if (data)
            {
                size_t* tmpControlBlock = reinterpret_cast<size_t*>(data) - 2;
                if (tmpControlBlock[0] != 0)
                {
                    return shared_ptr<T>(data, 0);
                }
            }

            reset();
            return shared_ptr<T>();
        }

    private:
        size_t increaseRefCount()
        {
            return controlBlock ? ++controlBlock[1] : ++(reinterpret_cast<size_t*>(data) - 2)[1];
        }

        size_t decreaseRefCount()
        {
            return controlBlock ? --controlBlock[1] : --(reinterpret_cast<size_t*>(data) - 2)[1];
        }

        T* data;
        size_t* controlBlock;
    };

    template <class T> class unique_ptr
    {
    private:
        struct move_ref
        {
            move_ref(T* data)
                : data(data)
            {}

            move_ref(unique_ptr<T>& data)
                : data(data.release())
            {}

            T* data;
        };

    public:
        template <class T> friend typename unique_ptr<T>::move_ref move(unique_ptr<T>& other);
        template <class T, class P1> friend typename unique_ptr<T>::move_ref make_unique(P1 p1);

        unique_ptr()
            : data(0)
        {}

        unique_ptr(T* data)
            : data(data)
        {}

        unique_ptr(move_ref const& other)
            : data(other.data)
        {}

        unique_ptr& operator = (move_ref const& other)
        {
            reset();
            data = other.data;

            return *this;
        }

        T*& operator = (T* other)
        {
            reset();
            data = other;

            return data;
        }

        ~unique_ptr()
        {
            reset();
        }

        T* get() const
        {
            return data;
        }

        T& operator*() const
        {
            return *data;
        }

        T* release()
        {
            T* tmpData = data;
            data = 0;
            return tmpData;
        }

        void reset()
        {
            if (data)
            {
                delete data;
            }

            data = 0;
        }

        T* operator ->() const
        {
            return data;
        }

    private:
        unique_ptr(unique_ptr& other)
            : data(other.data)
        {
            other.data = 0;
        }

        unique_ptr& operator = (unique_ptr& other) 
        {
            if (data)
            {
                delete data;
            }

            data = other.data;
            other.data = 0;

            return *this;
        }

        T* data;
    };

    template <class T> typename unique_ptr<T>::move_ref move(unique_ptr<T>& other)
    {
        return typename unique_ptr<T>::move_ref(other);
    }

    template <class T, class P1> typename unique_ptr<T>::move_ref make_unique(P1 p1) //would be overloaded
    {
        try
        {
            return typename unique_ptr<T>::move_ref(new T(p1));
        }
        catch (...)
        {
            return typename unique_ptr<T>::move_ref(0);
        }
    }
}