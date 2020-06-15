#pragma once

#include <memory>
#include <olm/olm.h>
#include <olm/sas.h>

namespace mtx {
namespace crypto {

struct OlmDeleter
{
        void operator()(OlmAccount *ptr) { delete[](reinterpret_cast<uint8_t *>(ptr)); }
        void operator()(OlmUtility *ptr) { delete[](reinterpret_cast<uint8_t *>(ptr)); }

        void operator()(OlmSession *ptr) { delete[](reinterpret_cast<uint8_t *>(ptr)); }
        void operator()(OlmOutboundGroupSession *ptr)
        {
                delete[](reinterpret_cast<uint8_t *>(ptr));
        }
        void operator()(OlmInboundGroupSession *ptr) { delete[](reinterpret_cast<uint8_t *>(ptr)); }
        void operator()(OlmSAS *ptr) { delete[](reinterpret_cast<uint8_t *>(ptr)); }
};

struct SASObject
{
        using olm_type = OlmSAS;

        static olm_type *allocate() { return olm_sas(new uint8_t[olm_sas_size()]); }
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
