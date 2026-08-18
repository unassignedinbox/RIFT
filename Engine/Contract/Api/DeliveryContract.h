//============================================================================================================================================
//                                                        DELIVERYCONTRACT.H
//============================================================================================================================================
// 🧩 One fallible call either delivers content or carries a refusal — the standalone slice of the Slate contract.

#pragma once

#include <cstdint>
#include <utility>

namespace Slate
{

//------------------------------------------------------------------------------------------------------------------------
//                                                         REFUSAL
//------------------------------------------------------------------------------------------------------------------------

/// 🧩 Why a delivery was refused. The closed reason set, mirrored from the engine contract.
/// tag   contract, nonallocating, nonthrowing
enum class RefusalReason : std::uint32_t
{
    ContentUnsupported   = 0u,   // [-] - the requested content cannot stand
    ExtentExhausted      = 1u,   // [-] - a bounded extent ran out
    CapabilityAbsent     = 2u,   // [-] - a capability was never constructed
    RefusalReasonCount   = 3u    // [-] - the closed count, never a reason
};

/// 🧩 One refusal: a reason plus the human run that states it.
/// tag   contract, nonallocating, nonthrowing
struct Refusal
{
    RefusalReason  Reason = RefusalReason::ContentUnsupported;   // [-] - the closed reason
    const char*    Run    = "";                                  // [-] - borrowed; states why
};

//------------------------------------------------------------------------------------------------------------------------
//                                                         DELIVERY
//------------------------------------------------------------------------------------------------------------------------

/// 🧩 One fallible call either delivers Content or carries a Refusal.
/// note  Standalone slice: same public spelling as the engine contract, without the precision transitivity
///       machinery — nothing in the standalone hosts consumes it yet.
/// tag   contract, nonallocating, nonthrowing
template <typename Content>
class Deliver
{
public:

    static Deliver Delivered(Content Arriving)   { return Deliver(Arriving, nullptr); }
    static Deliver Refuse(Refusal Declined)     { return Deliver(Content{}, &Declined); }

    bool        ContentPresent() const   { return Standing; }
    const Refusal&  Declined() const     { return Outcome; }
    Content&        Resolve()            { return Carried; }
    const Content&  Resolve() const      { return Carried; }

private:

    Deliver(Content Arriving, const Refusal* Declined)
        : Carried(std::move(Arriving)), Outcome(), Standing(Declined == nullptr)
    {
        if (Declined != nullptr)
            Outcome = *Declined;
    }

    Content   Carried;    // [-] - the delivered content when Standing
    Refusal   Outcome;    // [-] - the refusal when not Standing
    bool      Standing;   // [-] - whether content stands delivered
};

}   // namespace Slate
