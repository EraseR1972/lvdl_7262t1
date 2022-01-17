/**
 * Public header for Fragment
 * @file lv_fragment.h
 */

#ifndef LV_FRAGMENT_H
#define LV_FRAGMENT_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "../../../lv_conf_internal.h"

#if LV_USE_FRAGMENT

#include "../../../core/lv_obj.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

typedef struct _lv_fragment_manager_t lv_fragment_manager_t;

typedef struct lv_fragment_t lv_fragment_t;
typedef struct lv_fragment_class_t lv_fragment_class_t;
typedef struct lv_fragment_managed_states_t lv_fragment_managed_states_t;

struct lv_fragment_t {
    /**
     * Class of this fragment
     */
    const lv_fragment_class_t * cls;
    /**
     * Managed fragment states. If not null, then this fragment is managed.
     *
     * @warning Don't modify values inside this struct!
     */
    lv_fragment_managed_states_t * managed;
    /**
     * Child fragment manager
     */
    lv_fragment_manager_t * child_manager;
    /**
     * lv_obj returned by create_obj_cb
     */
    lv_obj_t * obj;

};

struct lv_fragment_class_t {
    /**
     * Constructor function for fragment class
     * @param self Fragment instance
     * @param args Arguments assigned by fragment manager
     */
    void (*constructor_cb)(struct lv_fragment_t * self, void * args);

    /**
     * Destructor function for fragment class
     * @param self Fragment instance, will be freed after this call
     */
    void (*destructor_cb)(struct lv_fragment_t * self);

    /**
     * Fragment attached to manager
     * @param self Fragment instance
     */
    void (*attached_cb)(struct lv_fragment_t * self);

    /**
     * Fragment detached from manager
     * @param self Fragment instance
     */
    void (*detached_cb)(struct lv_fragment_t * self);

    /**
     * Create objects
     * @param self Fragment instance
     * @param container Container of the objects should be created upon
     * @return Created object, NULL if multiple objects has been created
     */
    lv_obj_t * (*create_obj_cb)(struct lv_fragment_t * self, lv_obj_t * container);

    /**
     *
     * @param self Fragment instance
     * @param obj lv_obj returned by create_obj_cb
     */
    void (*obj_created_cb)(struct lv_fragment_t * self, lv_obj_t * obj);

    /**
     * Called before objects in the fragment will be deleted.
     *
     * @param self Fragment instance
     * @param obj object with this fragment
     */
    void (*obj_will_delete_cb)(struct lv_fragment_t * self, lv_obj_t * obj);

    /**
     * Called when the object created by fragment received `LV_EVENT_DELETE` event
     * @param self Fragment instance
     * @param obj object with this fragment
     */
    void (*obj_deleted_cb)(struct lv_fragment_t * self, lv_obj_t * obj);

    /**
     * Handle event
     * @param self Fragment instance
     * @param which User-defined ID of event
     * @param data1 User-defined data
     * @param data2 User-defined data
     */
    bool (*event_cb)(struct lv_fragment_t * self, int code, void * userdata);

    /**
     * *REQUIRED*: Allocation size of fragment
     */
    size_t instance_size;
};

/**
 * Fragment states
 */
typedef struct lv_fragment_managed_states_t  {
    /**
     * Class of the fragment
     */
    const lv_fragment_class_t * cls;
    /**
     * Manager the fragment attached to
     */
    lv_fragment_manager_t * manager;
    /**
     * Container object the fragment adding view to
     */
    lv_obj_t * const * container;
    /**
     * Fragment instance
     */
    lv_fragment_t * instance;
    /**
     * true between `create_obj_cb` and `obj_deleted_cb`
     */
    bool obj_created;
    /**
     * true before `lv_fragment_del_obj` is called. Don't touch any object if this is true
     */
    bool destroying_obj;
    /**
     * true if this fragment is in navigation stack that can be popped
     */
    bool in_stack;
} lv_fragment_managed_states_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create fragment manager instance
 * @param parent Parent fragment if this manager is placed inside another fragment, can be null.
 * @return Fragment manager instance
 */
lv_fragment_manager_t * lv_fragment_manager_create(lv_fragment_t * parent);

/**
 * Destroy fragment manager instance
 * @param manager Fragment manager instance
 */
void lv_fragment_manager_del(lv_fragment_manager_t * manager);

void lv_fragment_manager_create_obj(lv_fragment_manager_t * manager);

void lv_fragment_manager_del_obj(lv_fragment_manager_t * manager);

/**
 * Attach fragment to manager.
 * @param manager Fragment manager instance
 * @param fragment Fragment instance
 * @param container Pointer to container object for manager to add objects to
 */
void lv_fragment_manager_add(lv_fragment_manager_t * manager, lv_fragment_t * fragment, lv_obj_t * const * container);

/**
 * Detach and destroy fragment. If fragment is in navigation stack, remove from it.
 * @param manager Fragment manager instance
 * @param fragment Fragment instance
 */
void lv_fragment_manager_remove(lv_fragment_manager_t * manager, lv_fragment_t * fragment);

/**
 * Attach fragment to manager and add to navigation stack.
 * @param manager Fragment manager instance
 * @param fragment Fragment instance
 * @param container Pointer to container object for manager to add objects to
 */
void lv_fragment_manager_push(lv_fragment_manager_t * manager, lv_fragment_t * fragment, lv_obj_t * const * container);

/**
 * Remove the top-most fragment for stack
 * @param manager Fragment manager instance
 * @return true if there is fragment to pop
 */
bool lv_fragment_manager_pop(lv_fragment_manager_t * manager);

/**
 * Replace fragment. Old item in the stack will be removed.
 * @param manager Fragment manager instance
 * @param fragment Fragment instance
 */
void lv_fragment_manager_replace(lv_fragment_manager_t * manager, lv_fragment_t * fragment,
                                 lv_obj_t * const * container);

/**
 * Destroy obj in fragment, and recreate them.
 * @param manager Fragment manager instance
 * @param fragment Fragment instance
 */
void lv_fragment_manager_recreate_obj(lv_fragment_manager_t * manager, lv_fragment_t * fragment);

/**
 * Send event to top-most fragment
 * @param manager Fragment manager instance
 * @param code User-defined ID of event
 * @param userdata User-defined data
 * @return true if fragment returned true
 */
bool lv_fragment_manager_dispatch_event(lv_fragment_manager_t * manager, int code, void * userdata);

/**
 * Get stack size of this fragment manager
 * @param manager Fragment manager instance
 * @return Stack size of this fragment manager
 */
size_t lv_fragment_manager_get_stack_size(lv_fragment_manager_t * manager);

/**
 * Get top most fragment instance
 * @param manager Fragment manager instance
 * @return Top most fragment instance
 */
lv_fragment_t * lv_fragment_manager_get_top(lv_fragment_manager_t * manager);

lv_fragment_t * lv_fragment_manager_find_by_container(lv_fragment_manager_t * manager, const lv_obj_t * container);

/**
 * Get parent fragment
 * @param manager Fragment manager instance
 * @return Parent fragment instance
 */
lv_fragment_t * lv_fragment_manager_get_parent_fragment(lv_fragment_manager_t * manager);


/**
 * Create a fragment instance.
 *
 * @param cls Fragment class. This fragment must return non null object.
 * @param args Arguments assigned by fragment manager
 * @return Fragment instance
 */
lv_fragment_t * lv_fragment_create(const lv_fragment_class_t * cls, void * args);

/**
 * Destroy a fragment.
 * @param fragment Fragment instance.
 */
void lv_fragment_del(lv_fragment_t * fragment);

/**
 * Remove fragment from attached manager
 * @param fragment Fragment instance
 */
void lv_fragment_remove_self(lv_fragment_t * fragment);

/**
 * Get associated manager of this fragment
 * @param fragment Fragment instance
 * @return Fragment manager instance
 */
lv_fragment_manager_t * lv_fragment_get_manager(lv_fragment_t * fragment);

/**
 * Get container object of this fragment
 * @param fragment Fragment instance
 * @return Reference to container object
 */
lv_obj_t * const * lv_fragment_get_container(lv_fragment_t * fragment);

/**
 * Get parent fragment of this fragment
 * @param fragment Fragment instance
 * @return Parent fragment
 */
lv_fragment_t * lv_fragment_get_parent(lv_fragment_t * fragment);

/**
 * Create object by fragment.
 *
 * @param fragment Fragment instance.
 * @param container Container of the objects should be created upon.
 * @return Created object
 */
lv_obj_t * lv_fragment_create_obj(lv_fragment_t * fragment, lv_obj_t * container);

/**
 * Delete created object of a fragment
 *
 * @param fragment Fragment instance.
 */
void lv_fragment_del_obj(lv_fragment_t * fragment);

/**
 * Destroy obj in fragment, and recreate them.
 * @param fragment Fragment instance
 */
void lv_fragment_recreate_obj(lv_fragment_t * fragment);


/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_FRAGMENT*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_FRAGMENT_H*/
