//============================================================================================================================================
//                                                       INTERFACESEQUENCE.H
//============================================================================================================================================
// 🧩 The headless interface tick — adoption, pointer seating, tick open and seal — with no vendored spelling named.

#pragma once

#include "Contract/Api/PanelContract.h"

#include <cstdint>

namespace Slate
{
namespace Reference
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

    /// 🧩 Opens the borderless seat window inside an engine-owned tick — no frame of its own.
    /// note  🔴 The engine's InterfaceExchange owns NewFrame; the seat window is where the reference
    ///       panels' widgets stand, so they receive focus, keys and the pointer.
    /// out   Deliver  [-]  refuses when no context stands
    /// cost  ✔️
    /// tag   api, nonallocating, nonthrowing
    static Deliver<bool> OpenSeatWindow(double DisplayAlong, double DisplayAcross);

    /// 🧩 Closes the seat window opened by OpenSeatWindow.
    /// cost  ✔️
    /// tag   api, nonallocating, nonthrowing
    static void CloseSeatWindow();


};

}   // namespace Reference
}   // namespace Slate
