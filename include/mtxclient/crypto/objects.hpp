#pragma once

#include <memory>
#include <olm/olm.h>

namespace mtx {
namespace crypto {

struct OlmDeleter
{
        void operator()(OlmAccount *ptr) { operator delete(ptr, olm_account_size()); }
        void operator()(OlmUtility *ptr) { operator delete(ptr, olm_utility_size()); }

        void operator()(OlmSession *ptr) { operator delete(ptr, olm_session_size()); }
        void operator()(OlmOutboundGroupSession *ptr)
        {
                operator delete(ptr, olm_outbound_group_session_size());
        }
        void operator()(OlmInboundGroupSession *ptr)
        {
                operator delete(ptr, olm_inbound_group_session_size());
        }
};

struct UtilityObject
{
        using olm_type = OlmUtility;

        static olm_type *allocate() { return olm_utility(new uint8_t[olm_utility_size()]); }
};

struct AccountObject
{
        using olm_type = OlmAccount;

        static olm_type *allocate() { return olm_account(new uint8_t[olm_account_size()]); }

        static size_t pickle_length(olm_type *account)
        {
                return olm_pickle_account_length(account);
        }

        static size_t pickle(olm_type *account,
                             void const *key,
                             size_t key_length,
                             void *pickled,
                             size_t pickled_length)
        {
                return olm_pickle_account(account, key, key_length, pickled, pickled_length);
        }

        static size_t unpickle(olm_type *account,
                               void const *key,
                               size_t key_length,
                               void *pickled,
                               size_t pickled_length)
        {
                return olm_unpickle_account(account, key, key_length, pickled, pickled_length);
        }
};

struct SessionObject
{
        using olm_type = OlmSession;

        static olm_type *allocate() { return olm_session(new uint8_t[olm_session_size()]); }

        static size_t pickle_length(olm_type *session)
        {
                return olm_pickle_session_length(session);
        }

        static size_t pickle(olm_type *session,
                             void const *key,
                             size_t key_length,
                             void *pickled,
                             size_t pickled_length)
        {
                return olm_pickle_session(session, key, key_length, pickled, pickled_length);
        }

        static size_t unpickle(olm_type *session,
                               void const *key,
                               size_t key_length,
                               void *pickled,
                               size_t pickled_length)
        {
                return olm_unpickle_session(session, key, key_length, pickled, pickled_length);
        }
};

struct InboundSessionObject
{
        using olm_type = OlmInboundGroupSession;

        static olm_type *allocate()
        {
                return olm_inbound_group_session(new uint8_t[olm_inbound_group_session_size()]);
        }

        static size_t pickle_length(olm_type *session)
        {
                return olm_pickle_inbound_group_session_length(session);
        }

        static size_t pickle(olm_type *session,
                             void const *key,
                             size_t key_length,
                             void *pickled,
                             size_t pickled_length)
        {
                return olm_pickle_inbound_group_session(
                  session, key, key_length, pickled, pickled_length);
        }

        static size_t unpickle(olm_type *session,
                               void const *key,
                               size_t key_length,
                               void *pickled,
                               size_t pickled_length)
        {
                return olm_unpickle_inbound_group_session(
                  session, key, key_length, pickled, pickled_length);
        }
};

struct OutboundSessionObject
{
        using olm_type = OlmOutboundGroupSession;

        static olm_type *allocate()
        {
                return olm_outbound_group_session(new uint8_t[olm_outbound_group_session_size()]);
        }

        static size_t pickle_length(olm_type *session)
        {
                return olm_pickle_outbound_group_session_length(session);
        }

        static size_t pickle(olm_type *session,
                             void const *key,
                             size_t key_length,
                             void *pickled,
                             size_t pickled_length)
        {
                return olm_pickle_outbound_group_session(
                  session, key, key_length, pickled, pickled_length);
        }

        static size_t unpickle(olm_type *session,
                               void const *key,
                               size_t key_length,
                               void *pickled,
                               size_t pickled_length)
        {
                return olm_unpickle_outbound_group_session(
                  session, key, key_length, pickled, pickled_length);
        }
};

template<class T>
std::unique_ptr<typename T::olm_type, OlmDeleter>
create_olm_object()
{
        return std::unique_ptr<typename T::olm_type, OlmDeleter>(T::allocate());
}
}
}
