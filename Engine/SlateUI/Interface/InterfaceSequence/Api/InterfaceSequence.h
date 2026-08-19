//============================================================================================================================================
//                                                       INTERFACESEQUENCE.H
//============================================================================================================================================
// 🧩 The headless interface tick — adoption, pointer seating, tick open and seal — with no vendored spelling named.

#pragma once

#include "Contract/Api/PanelContract.h"
#include "SlateUI/Interface/IconDepot/Api/IconDepot.h"

#include <cstdint>

namespace Rift
{

//------------------------------------------------------------------------------------------------------------------------
//                                                    THE TICK SEQUENCE
//------------------------------------------------------------------------------------------------------------------------

/// 🧩 The interface sequence a headless host walks: adopt once, seat the pointer, open a tick, present
///       through the seam, seal the tick and take the recorded draw data for the raster codec.
/// note  🔴 The Api names no vendored spelling — the host side of the partition stays clean; the Source,
///       seated inside SlateUI, is the one place the vendored context is addressed.
/// tag   contract, nonallocating, nonthrowing
struct InterfaceSequence
{
    /// 🧩 Adopts the vendored interface context at the declared display extent, with the default
    ///       typeface seated at three crisp sizes and the borderless host chrome styled.
    /// out   Deliver  [-]  refuses with CapabilityAbsent when the vendored context refuses to stand
    /// cost  🚩
    /// tag   api, nonthrowing
    static Deliver<bool> Adopt(double DisplayAlong, double DisplayAcross);

    /// 🧩 Seats the pointer at the declared ordinates for the next tick.
    /// cost  ✔️
    /// tag   api, nonallocating, nonthrowing
    static void SeatPointer(float Along, float Across);

    /// 🧩 Seats one primary press edge — the button stands down until the host seats it elsewhere.
    /// cost  ✔️
    /// tag   api, nonallocating, nonthrowing
    static void SeatPrimaryPress();

    /// 🧩 Opens one tick: the frame begins and the borderless host window stands, ready for the seam.
    /// out   Deliver  [-]  refuses when no context stands adopted
    /// cost  ✔️
    /// tag   api, nonallocating, nonthrowing
    static Deliver<bool> OpenTick();

    /// 🧩 Seals the tick and hands the recorded draw data, untyped, to the raster codec.
    /// out   the recorded draw data of the sealed tick; nullptr when nothing stands
    /// cost  ✔️
    /// tag   api, nonallocating, nonthrowing
    static void* SealTick();

    /// 🧩 The recorded draw data of the last sealed tick — the backends' present input.
    /// cost  ✔️
    /// tag   api, nonallocating, nonthrowing
    static void* SealTickDrawData();

    /// 🧩 Dismisses the adopted context.
    /// cost  ✔️
    /// tag   api, nonallocating, nonthrowing
    static void Dismiss();

    /// 🧩 Seats the platform's unhandled-exception reporter, so a silent fault names itself on the console.
    /// note  Windows-only; a no-op elsewhere. Reports the exception ordinal and the faulting address.
    /// cost  ✔️
    /// tag   api, nonthrowing
    static void SeatFaultReporter();

    /// 🧩 Names the standing stage on the unbuffered error stream — the trace a silent fault leaves behind.
    /// cost  ✔️
    /// tag   api, nonallocating, nonthrowing
    static void NameStage(const char* StageRun);

    /// 🧩 Adopts the vendored context behind a real platform window — the interactive seat.
    /// note  🔴 The window translation units live inside SlateUI, the one unit permitted the vendored
    ///       window and interface backends; the host drives only these calls.
    /// out   Deliver  [-]  refuses with CapabilityAbsent when the window refuses to open
    /// cost  🚩
    /// tag   api, nonthrowing
    static Deliver<bool> AdoptWindowed(double DisplayAlong, double DisplayAcross, const char* TitleRun);

    /// 🧩 Whether the window still stands open.
    /// cost  ✔️
    /// tag   api, nonallocating, nonthrowing
    static bool WindowStanding();

    /// 🧩 Begins one windowed tick — events polled, the frame opens, the host window stands.
    /// out   Deliver  [-]  refuses when no window stands
    /// cost  ✔️
    /// tag   api, nonallocating, nonthrowing
    static Deliver<bool> BeginWindowTick();

    /// 🧩 Ends the windowed tick — the frame seals, the backends present, the buffer swaps.
    /// cost  ✔️
    /// tag   api, nonallocating, nonthrowing
    static void EndWindowTick();

    /// 🧩 Uploads the glyph depot's raster once and adopts the platform picture identity.
    /// cost  🚩
    /// tag   api, nonthrowing
    static void SeatGlyphPicture(const IconDepot& Depot);

    /// 🧩 Dismisses the windowed seat.
    /// cost  ✔️
    /// tag   api, nonallocating, nonthrowing
    static void DismissWindowed();
};

}   // namespace Rift
