#include <node.h>
using namespace v8;
template <class T> class ABase {
public:
    /**
     * If non-empty, destroy the underlying storage cell
     * IsEmpty() will return true after this call.
     */
    V8_INLINE void Reset();
    /**
     * If non-empty, destroy the underlying storage cell
     * and create a new one with the contents of other if other is non empty
     */
    template <class S>
    V8_INLINE void Reset(Isolate* isolate, const Local<S>& other);
  
    /**
     * If non-empty, destroy the underlying storage cell
     * and create a new one with the contents of other if other is non empty
     */
    template <class S>
    V8_INLINE void Reset(Isolate* isolate, const ABase<S>& other);
  
    V8_INLINE bool IsEmpty() const { return val_ == NULL; }
    V8_INLINE void Empty() { val_ = 0; }
  
    V8_INLINE Local<T> Get(Isolate* isolate) const {
      return Local<T>();
    }
  
    template <class S>
    V8_INLINE bool operator==(const ABase<S>& that) const {
      internal::Object** a = reinterpret_cast<internal::Object**>(this->val_);
      internal::Object** b = reinterpret_cast<internal::Object**>(that.val_);
      if (a == NULL) return b == NULL;
      if (b == NULL) return false;
      return *a == *b;
    }
  
    template <class S>
    V8_INLINE bool operator==(const Local<S>& that) const {
      internal::Object** a = reinterpret_cast<internal::Object**>(this->val_);
      internal::Object** b = reinterpret_cast<internal::Object**>(that.val_);
      if (a == NULL) return b == NULL;
      if (b == NULL) return false;
      return *a == *b;
    }
  
    template <class S>
    V8_INLINE bool operator!=(const ABase<S>& that) const {
      return !operator==(that);
    }
  
    template <class S>
    V8_INLINE bool operator!=(const Local<S>& that) const {
      return !operator==(that);
    }
  
    /**
     *  Install a finalization callback on this object.
     *  NOTE: There is no guarantee as to *when* or even *if* the callback is
     *  invoked. The invocation is performed solely on a best effort basis.
     *  As always, GC-based finalization should *not* be relied upon for any
     *  critical form of resource management!
     */
    template <typename P>
    V8_INLINE void SetWeak(P* parameter,
                           typename WeakCallbackInfo<P>::Callback callback,
                           WeakCallbackType type);
  
    /**
     * Turns this handle into a weak phantom handle without finalization callback.
     * The handle will be reset automatically when the garbage collector detects
     * that the object is no longer reachable.
     * A related function Isolate::NumberOfPhantomHandleResetsSinceLastCall
     * returns how many phantom handles were reset by the garbage collector.
     */
    V8_INLINE void SetWeak();
  
    template<typename P>
    V8_INLINE P* ClearWeak();
  
    // TODO(dcarney): remove this.
    V8_INLINE void ClearWeak() { ClearWeak<void>(); }
  
    /**
     * Allows the embedder to tell the v8 garbage collector that a certain object
     * is alive. Only allowed when the embedder is asked to trace its heap by
     * EmbedderHeapTracer.
     */
    V8_INLINE void RegisterExternalReference(Isolate* isolate) const;
  
    /**
     * Marks the reference to this object independent. Garbage collector is free
     * to ignore any object groups containing this object. Weak callback for an
     * independent handle should not assume that it will be preceded by a global
     * GC prologue callback or followed by a global GC epilogue callback.
     */
    V8_INLINE void MarkIndependent();
  
    /**
     * Marks the reference to this object as active. The scavenge garbage
     * collection should not reclaim the objects marked as active.
     * This bit is cleared after the each garbage collection pass.
     */
    V8_INLINE void MarkActive();
  
    V8_INLINE bool IsIndependent() const;
  
    /** Checks if the handle holds the only reference to an object. */
    V8_INLINE bool IsNearDeath() const;
  
    /** Returns true if the handle's reference is weak.  */
    V8_INLINE bool IsWeak() const;
  
    /**
     * Assigns a wrapper class ID to the handle. See RetainedObjectInfo interface
     * description in v8-profiler.h for details.
     */
    V8_INLINE void SetWrapperClassId(uint16_t class_id);
  
    /**
     * Returns the class ID previously assigned to this handle or 0 if no class ID
     * was previously assigned.
     */
    V8_INLINE uint16_t WrapperClassId() const;
  
    ABase(const ABase& other) = delete;  // NOLINT
    void operator=(const ABase&) = delete;
  
   private:
  
    friend class Utils;
    template<class F> friend class Local;
    template<class F1, class F2> friend class Persistent;
    template <class F>
    friend class Global;
    template<class F> friend class ABase;
    template<class F> friend class ReturnValue;
    template <class F1, class F2, class F3>
    friend class PersistentValueMapBase;
    template<class F1, class F2> friend class PersistentValueVector;
    friend class Object;
  
    explicit V8_INLINE ABase(T* val) : val_(val) {}
    V8_INLINE static T* New(Isolate* isolate, T* that);
  
    T* val_;
};
template<typename T>
class A: public ABase<T>{

};
//typedef Persistent<Function> a;
typedef A<Function> a;
template<typename T>
struct B{
    static a constructor;
};
template<> a B<int>::constructor;

void test(){
    B<int>::constructor.Get(Isolate::GetCurrent());
}