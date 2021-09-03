#pragma once

/// @file
/// @brief Wrappers around the various Olm types.
///
/// The wrappers implement RAII semantics, so you don't need to free stuff manually.

#include <memory>
#include <olm/olm.h>
#include <olm/pk.h>
#include <olm/sas.h>

namespace mtx {
namespace crypto {

/// @brief Deleter type to pass as a template argument to most smart pointers.
///
/// Can be used like so:
///
/// ```{.cpp}
/// std::unique_ptr<OlmAccount, OlmDeleter> olmAccount = new uint8_t[olm_account_size()];
/// ```
///
/// In general the coresponding *Object type should be preffered as a wapper, for example:
///
/// ```{.cpp}
/// SASPtr sas = create_olm_object<SASObject>();
/// ```
struct OlmDeleter
{
    void operator()(OlmAccount *ptr)
    {
        olm_clear_account(ptr);
        delete[](reinterpret_cast<uint8_t *>(ptr));
    }
    void operator()(OlmUtility *ptr)
    {
        olm_clear_utility(ptr);
        delete[](reinterpret_cast<uint8_t *>(ptr));
    }

    void operator()(OlmPkDecryption *ptr)
    {
        olm_clear_pk_decryption(ptr);
        delete[](reinterpret_cast<uint8_t *>(ptr));
    }
    void operator()(OlmPkEncryption *ptr)
    {
        olm_clear_pk_encryption(ptr);
        delete[](reinterpret_cast<uint8_t *>(ptr));
    }
    void operator()(OlmPkSigning *ptr)
    {
        olm_clear_pk_signing(ptr);
        delete[](reinterpret_cast<uint8_t *>(ptr));
    }

    void operator()(OlmSession *ptr)
    {
        olm_clear_session(ptr);
        delete[](reinterpret_cast<uint8_t *>(ptr));
    }
    void operator()(OlmOutboundGroupSession *ptr)
    {
        olm_clear_outbound_group_session(ptr);
        delete[](reinterpret_cast<uint8_t *>(ptr));
    }
    void operator()(OlmInboundGroupSession *ptr)
    {
        olm_clear_inbound_group_session(ptr);
        delete[](reinterpret_cast<uint8_t *>(ptr));
    }
    void operator()(OlmSAS *ptr)
    {
        olm_clear_sas(ptr);
        delete[](reinterpret_cast<uint8_t *>(ptr));
    }
};

//! Olm type for Short Authentication Strings.
struct SASObject
{
    using olm_type = OlmSAS;

    static olm_type *allocate() { return olm_sas(new uint8_t[olm_sas_size()]); }
};

//! Wrapper for the Olm utility object.
struct UtilityObject
{
    using olm_type = OlmUtility;

    static olm_type *allocate() { return olm_utility(new uint8_t[olm_utility_size()]); }
};

//! Wrapper for the olm object to do Private Key Decryption.
struct PkDecryptionObject
{
    using olm_type = OlmPkDecryption;

    static olm_type *allocate() { return olm_pk_decryption(new uint8_t[olm_pk_decryption_size()]); }
};

//! Wrapper for the olm object to do Private Key Decryption.
struct PkEncryptionObject
{
    using olm_type = OlmPkEncryption;

    static olm_type *allocate() { return olm_pk_encryption(new uint8_t[olm_pk_encryption_size()]); }
};

//! Wrapper for the olm object to do Private Key Signing.
struct PkSigningObject
{
    using olm_type = OlmPkSigning;

    static olm_type *allocate() { return olm_pk_signing(new uint8_t[olm_pk_signing_size()]); }
};

//! Wrapper for the olm account object.
struct AccountObject
{
    using olm_type = OlmAccount;

    static olm_type *allocate() { return olm_account(new uint8_t[olm_account_size()]); }

    static size_t pickle_length(olm_type *account) { return olm_pickle_account_length(account); }

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

//! Wrapper around olm sessions used for to device encryption.
struct SessionObject
{
    using olm_type = OlmSession;

    static olm_type *allocate() { return olm_session(new uint8_t[olm_session_size()]); }

    static size_t pickle_length(olm_type *session) { return olm_pickle_session_length(session); }

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

//! Wrapper around the olm object for inbound group sessions used to decrypt messages in matrix
//! rooms.
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
        return olm_pickle_inbound_group_session(session, key, key_length, pickled, pickled_length);
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

//! Wrapper around the outbound olm session object used to encrypt outbound group messages in matrix
//! rooms.
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
        return olm_pickle_outbound_group_session(session, key, key_length, pickled, pickled_length);
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

//! Allocates an olm object using the mtxclient wrapper type.
template<class T>
std::unique_ptr<typename T::olm_type, OlmDeleter>
create_olm_object()
{
    return std::unique_ptr<typename T::olm_type, OlmDeleter>(T::allocate());
}

using OlmSessionPtr           = std::unique_ptr<OlmSession, OlmDeleter>;
using OutboundGroupSessionPtr = std::unique_ptr<OlmOutboundGroupSession, OlmDeleter>;
using InboundGroupSessionPtr  = std::unique_ptr<OlmInboundGroupSession, OlmDeleter>;
using SASPtr                  = std::unique_ptr<OlmSAS, OlmDeleter>;
}
}
