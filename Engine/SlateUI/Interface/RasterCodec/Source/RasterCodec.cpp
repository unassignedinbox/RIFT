#define _CRT_SECURE_NO_WARNINGS
//============================================================================================================================================
//                                                            RASTERCODEC.CPP
//============================================================================================================================================
// 🧩 Scanline coverage, bilinear texture reads, straight-alpha source-over — the whole pipeline is arithmetic on the byte extent.

#include "SlateUI/Interface/RasterCodec/Api/RasterCodec.h"

#include "imgui.h"

#include <cmath>
#include <cstdio>
#include <cstring>
#include <cstdint>
#include <filesystem>
#include <system_error>

namespace Slate
{
namespace Reference
{

Deliver<bool> RasterCodec::SeatAtlas(void* Identity)
{
    ImGuiIO& VendorIO = ImGui::GetIO();
    if (VendorIO.Fonts == nullptr)
        return Deliver<bool>::Refuse({ RefusalReason::CapabilityAbsent, "no font atlas stands constructed" });

    unsigned char* Ordinates = nullptr;
    int Along = 0, Across = 0;
    VendorIO.Fonts->GetTexDataAsRGBA32(&Ordinates, &Along, &Across);
    if (Ordinates == nullptr)
        return Deliver<bool>::Refuse({ RefusalReason::ExtentExhausted, "the atlas resolved to no ordinates" });

    AtlasSeat         = Identity;
    AtlasAlongExtent  = static_cast<std::uint32_t>(Along);
    AtlasAcrossExtent = static_cast<std::uint32_t>(Across);
    AtlasOrdinates.assign(Ordinates, Ordinates + static_cast<std::size_t>(Along) * Across * 4u);
    VendorIO.Fonts->SetTexID(static_cast<ImTextureID>(reinterpret_cast<std::uintptr_t>(Identity)));
    return Deliver<bool>::Delivered(true);
}

namespace
{

/// 🧩 One resolved texture the rasterizer samples.
/// tag   internal
struct ResolvedTexture
{
    const std::uint8_t* Ordinates    = nullptr;   // [-] - borrowed RGBA
    std::uint32_t       AlongExtent  = 1u;        // [px]
    std::uint32_t       AcrossExtent = 1u;        // [px]
    bool                Standing     = false;     // [-] - a real texture, else the white constant
};

/// 🧩 One bilinear texture read, clamped to the extent.
/// cost  🚩
void SampleTexture(const ResolvedTexture& Texture, float Along, float Across, float (&Ordinate)[4])
{
    if (!Texture.Standing)
    {
        Ordinate[0] = Ordinate[1] = Ordinate[2] = Ordinate[3] = 255.0f;
        return;
    }

    const float X = Along < 0.0f ? 0.0f : (Along > Texture.AlongExtent - 1.0f ? Texture.AlongExtent - 1.0f : Along);
    const float Y = Across < 0.0f ? 0.0f : (Across > Texture.AcrossExtent - 1.0f ? Texture.AcrossExtent - 1.0f : Across);

    const std::uint32_t X0 = static_cast<std::uint32_t>(X);
    const std::uint32_t Y0 = static_cast<std::uint32_t>(Y);
    const std::uint32_t X1 = X0 + 1u < Texture.AlongExtent ? X0 + 1u : X0;
    const std::uint32_t Y1 = Y0 + 1u < Texture.AcrossExtent ? Y0 + 1u : Y0;
    const float FractionX = X - static_cast<float>(X0);
    const float FractionY = Y - static_cast<float>(Y0);

    for (std::uint32_t Component = 0u; Component < 4u; ++Component)
    {
        const float A = static_cast<float>(Texture.Ordinates[(static_cast<std::size_t>(Y0) * Texture.AlongExtent + X0) * 4u + Component]);
        const float B = static_cast<float>(Texture.Ordinates[(static_cast<std::size_t>(Y0) * Texture.AlongExtent + X1) * 4u + Component]);
        const float C = static_cast<float>(Texture.Ordinates[(static_cast<std::size_t>(Y1) * Texture.AlongExtent + X0) * 4u + Component]);
        const float D = static_cast<float>(Texture.Ordinates[(static_cast<std::size_t>(Y1) * Texture.AlongExtent + X1) * 4u + Component]);
        Ordinate[Component] = (A * (1.0f - FractionX) + B * FractionX) * (1.0f - FractionY)
                            + (C * (1.0f - FractionX) + D * FractionX) * FractionY;
    }
}

}   // namespace

void RasterCodec::Rasterize(const void* RecordedDrawData, PixelSpace& Extent)
{
    const ImDrawData* Recorded = static_cast<const ImDrawData*>(RecordedDrawData);

    // ①① The atlas may have baked late in the tick — resolve its ordinates fresh, every translation.
    if (AtlasSeat != nullptr)
    {
        unsigned char* Ordinates = nullptr;
        int Along = 0, Across = 0;
        ImGui::GetIO().Fonts->GetTexDataAsRGBA32(&Ordinates, &Along, &Across);
        if (Ordinates != nullptr)
        {
            AtlasAlongExtent  = static_cast<std::uint32_t>(Along);
            AtlasAcrossExtent = static_cast<std::uint32_t>(Across);
            AtlasOrdinates.assign(Ordinates, Ordinates + static_cast<std::size_t>(Along) * Across * 4u);
        }
    }
    Extent.Ordinates.assign(static_cast<std::size_t>(Extent.AcrossExtent) * Extent.AlongExtent * 4u, 0u);

    const auto ResolveTexture = [&](ImTextureID Identity) -> ResolvedTexture
    {
        if (reinterpret_cast<void*>(static_cast<std::uintptr_t>(Identity)) == AtlasSeat && AtlasSeat != nullptr)
            return { AtlasOrdinates.data(), AtlasAlongExtent, AtlasAcrossExtent, true };
        for (const PictureDeclaration& Picture : Seated)
            if (Picture.Identity == reinterpret_cast<void*>(static_cast<std::uintptr_t>(Identity)))
                return { Picture.Ordinates, Picture.AlongExtent, Picture.AcrossExtent, true };
        return { nullptr, 1u, 1u, false };
    };

    for (int ListOrdinal = 0; ListOrdinal < Recorded->CmdListsCount; ++ListOrdinal)
    {
        const ImDrawList* List = Recorded->CmdLists[ListOrdinal];
        const ImDrawVert* VertexRun = List->VtxBuffer.Data;
        const ImDrawIdx*  IndexRun  = List->IdxBuffer.Data;

        for (int CommandOrdinal = 0; CommandOrdinal < List->CmdBuffer.Size; ++CommandOrdinal)
        {
            const ImDrawCmd& Command = List->CmdBuffer[CommandOrdinal];
            if (Command.UserCallback != nullptr)
                continue;

            const ResolvedTexture Texture = ResolveTexture(Command.GetTexID());
            const float ClipLeastAlong  = Command.ClipRect.x < 0.0f ? 0.0f : Command.ClipRect.x;
            const float ClipLeastAcross = Command.ClipRect.y < 0.0f ? 0.0f : Command.ClipRect.y;
            const float ClipMostAlong   = Command.ClipRect.z > static_cast<float>(Extent.AlongExtent)  ? static_cast<float>(Extent.AlongExtent)  : Command.ClipRect.z;
            const float ClipMostAcross  = Command.ClipRect.w > static_cast<float>(Extent.AcrossExtent) ? static_cast<float>(Extent.AcrossExtent) : Command.ClipRect.w;

            for (unsigned int Triangle = 0u; Triangle < Command.ElemCount; Triangle += 3u)
            {
                const ImDrawVert& VertexA = VertexRun[IndexRun[Command.IdxOffset + Triangle + 0u]];
                const ImDrawVert& VertexB = VertexRun[IndexRun[Command.IdxOffset + Triangle + 1u]];
                const ImDrawVert& VertexC = VertexRun[IndexRun[Command.IdxOffset + Triangle + 2u]];

                // ① Signed edge products — the coverage test, orientation-corrected.
                const float Area = (VertexB.pos.x - VertexA.pos.x) * (VertexC.pos.y - VertexA.pos.y)
                                 - (VertexC.pos.x - VertexA.pos.x) * (VertexB.pos.y - VertexA.pos.y);
                if (std::fabs(Area) < 1.0e-9f)
                    continue;
                const float Orientation = Area > 0.0f ? 1.0f : -1.0f;

                float LeastAlong  = VertexA.pos.x, MostAlong  = VertexA.pos.x;
                float LeastAcross = VertexA.pos.y, MostAcross = VertexA.pos.y;
                const ImDrawVert* Corners[3] = { &VertexA, &VertexB, &VertexC };
                for (const ImDrawVert* Corner : Corners)
                {
                    LeastAlong  = LeastAlong  < Corner->pos.x ? LeastAlong  : Corner->pos.x;
                    MostAlong   = MostAlong   > Corner->pos.x ? MostAlong   : Corner->pos.x;
                    LeastAcross = LeastAcross < Corner->pos.y ? LeastAcross : Corner->pos.y;
                    MostAcross  = MostAcross  > Corner->pos.y ? MostAcross  : Corner->pos.y;
                }
                LeastAlong  = LeastAlong  > ClipLeastAlong  ? LeastAlong  : ClipLeastAlong;
                MostAlong   = MostAlong   < ClipMostAlong   ? MostAlong   : ClipMostAlong;
                LeastAcross = LeastAcross > ClipLeastAcross ? LeastAcross : ClipLeastAcross;
                MostAcross  = MostAcross  < ClipMostAcross  ? MostAcross  : ClipMostAcross;

                const std::int32_t AcrossBegin = static_cast<std::int32_t>(LeastAcross);
                const std::int32_t AcrossEnd   = static_cast<std::int32_t>(MostAcross + 1.0f);
                const std::int32_t AlongBegin  = static_cast<std::int32_t>(LeastAlong);
                const std::int32_t AlongEnd    = static_cast<std::int32_t>(MostAlong + 1.0f);

                for (std::int32_t Across = AcrossBegin; Across < AcrossEnd && Across < static_cast<std::int32_t>(Extent.AcrossExtent); ++Across)
                {
                    if (Across < 0)
                        continue;
                    for (std::int32_t Along = AlongBegin; Along < AlongEnd && Along < static_cast<std::int32_t>(Extent.AlongExtent); ++Along)
                    {
                        if (Along < 0)
                            continue;

                        const float CentreAlong  = static_cast<float>(Along) + 0.5f;
                        const float CentreAcross = static_cast<float>(Across) + 0.5f;

                        const float WeightA = ((VertexB.pos.x - VertexA.pos.x) * (CentreAcross - VertexA.pos.y)
                                             - (CentreAlong - VertexA.pos.x) * (VertexB.pos.y - VertexA.pos.y)) * Orientation;
                        const float WeightB = ((VertexC.pos.x - VertexB.pos.x) * (CentreAcross - VertexB.pos.y)
                                             - (CentreAlong - VertexB.pos.x) * (VertexC.pos.y - VertexB.pos.y)) * Orientation;
                        const float WeightC = ((VertexA.pos.x - VertexC.pos.x) * (CentreAcross - VertexC.pos.y)
                                             - (CentreAlong - VertexC.pos.x) * (VertexA.pos.y - VertexC.pos.y)) * Orientation;
                        if (WeightA < 0.0f || WeightB < 0.0f || WeightC < 0.0f)
                            continue;

                        const float InverseArea = 1.0f / Area * Orientation;
                        const float ShareA = WeightA * InverseArea;
                        const float ShareB = WeightB * InverseArea;
                        const float ShareC = WeightC * InverseArea;

                        float Colour[4];
                        for (std::uint32_t Component = 0u; Component < 4u; ++Component)
                        {
                            const float ChannelA = static_cast<float>((VertexA.col >> (Component * 8u)) & 0xFFu);
                            const float ChannelB = static_cast<float>((VertexB.col >> (Component * 8u)) & 0xFFu);
                            const float ChannelC = static_cast<float>((VertexC.col >> (Component * 8u)) & 0xFFu);
                            Colour[Component] = ChannelA * ShareA + ChannelB * ShareB + ChannelC * ShareC;
                        }

                        const float SampleAlong = VertexA.uv.x * ShareA + VertexB.uv.x * ShareB + VertexC.uv.x * ShareC;
                        const float SampleAcross = VertexA.uv.y * ShareA + VertexB.uv.y * ShareB + VertexC.uv.y * ShareC;
                        float TextureOrdinate[4];
                        SampleTexture(Texture, SampleAlong, SampleAcross, TextureOrdinate);

                        // ② Modulate, then source-over in straight alpha onto the extent.
                        const float SourceAlpha = (Colour[3] / 255.0f) * (TextureOrdinate[3] / 255.0f);
                        if (SourceAlpha <= 0.0f)
                            continue;

                        std::uint8_t* Seat = &Extent.Ordinates[(static_cast<std::size_t>(Across) * Extent.AlongExtent + Along) * 4u];
                        const float StandingAlpha = Seat[3] / 255.0f;
                        const float BlendedAlpha = SourceAlpha + StandingAlpha * (1.0f - SourceAlpha);

                        for (std::uint32_t Component = 0u; Component < 3u; ++Component)
                        {
                            const float SourceChannel = (Colour[Component] / 255.0f) * (TextureOrdinate[Component] / 255.0f) * 255.0f;
                            const float StandingChannel = Seat[Component];
                            const float Blended = SourceAlpha * SourceChannel + StandingAlpha * (1.0f - SourceAlpha) * StandingChannel;
                            Seat[Component] = static_cast<std::uint8_t>(BlendedAlpha > 0.0f ? Blended / BlendedAlpha + 0.5f : 0u);
                        }
                        Seat[3] = static_cast<std::uint8_t>(BlendedAlpha * 255.0f + 0.5f);
                    }
                }
            }
        }
    }
}

Deliver<bool> RasterCodec::WriteRawDump(const PixelSpace& Extent, const char* Path)
{
    std::FILE* Stream = std::fopen(Path, "wb");
    if (Stream == nullptr)
        return Deliver<bool>::Refuse({ RefusalReason::ExtentExhausted, "the raw dump path refused to open" });

    std::fwrite("RIFTRAW1", 1u, 8u, Stream);
    const std::uint32_t Header[2] = { Extent.AlongExtent, Extent.AcrossExtent };
    std::fwrite(Header, sizeof(std::uint32_t), 2u, Stream);
    std::fwrite(Extent.Ordinates.data(), 1u, Extent.Ordinates.size(), Stream);
    std::fclose(Stream);
    return Deliver<bool>::Delivered(true);
}

namespace
{

/// 🧩 The CRC-32 of one run, polynomial 0xEDB88320, table-driven.
/// cost  🚩
std::uint32_t CyclicRedundancyCheck(const std::uint8_t* Run, std::size_t Extent)
{
    static std::uint32_t Standing[256];
    static bool Seated = false;
    if (!Seated)
    {
        for (std::uint32_t Ordinal = 0u; Ordinal < 256u; ++Ordinal)
        {
            std::uint32_t Remainder = Ordinal;
            for (int Cycle = 0; Cycle < 8; ++Cycle)
                Remainder = (Remainder & 1u) ? (Remainder >> 1) ^ 0xEDB88320u : (Remainder >> 1);
            Standing[Ordinal] = Remainder;
        }
        Seated = true;
    }
    std::uint32_t Check = 0xFFFFFFFFu;
    for (std::size_t Ordinal = 0u; Ordinal < Extent; ++Ordinal)
        Check = Standing[(Check ^ Run[Ordinal]) & 0xFFu] ^ (Check >> 8);
    return Check ^ 0xFFFFFFFFu;
}

/// 🧩 The Adler-32 of one run.
/// cost  🚩
std::uint32_t AdlerThirtyTwo(const std::uint8_t* Run, std::size_t Extent)
{
    std::uint32_t Lower = 1u;
    std::uint32_t Upper = 0u;
    for (std::size_t Ordinal = 0u; Ordinal < Extent; ++Ordinal)
    {
        Lower = (Lower + Run[Ordinal]) % 65521u;
        Upper = (Upper + Lower) % 65521u;
    }
    return (Upper << 16) | Lower;
}

/// 🧩 Writes one big-endian ordinate.
/// cost  ✔️
void WriteBigEndian(std::FILE* Stream, std::uint32_t Ordinate)
{
    const std::uint8_t Run[4] = { static_cast<std::uint8_t>(Ordinate >> 24), static_cast<std::uint8_t>(Ordinate >> 16),
                                  static_cast<std::uint8_t>(Ordinate >> 8), static_cast<std::uint8_t>(Ordinate) };
    std::fwrite(Run, 1u, 4u, Stream);
}

}   // namespace

Deliver<bool> RasterCodec::WritePortableNetworkGraphic(const PixelSpace& Extent, const char* Path)
{
    // ① The directories along the path are created when absent.
    std::filesystem::path Declared(Path);
    if (Declared.has_parent_path() && !Declared.parent_path().empty())
    {
        std::error_code Outcome;
        std::filesystem::create_directories(Declared.parent_path(), Outcome);   // 📝 failures surface at fopen below
    }

    std::FILE* Stream = std::fopen(Path, "wb");
    if (Stream == nullptr)
        return Deliver<bool>::Refuse({ RefusalReason::ExtentExhausted, "the proof path refused to open" });

    // ② Scanlines — filter 0, one stride each.
    const std::size_t Stride = static_cast<std::size_t>(Extent.AlongExtent) * 4u;
    std::vector<std::uint8_t> Scanlines((static_cast<std::size_t>(Extent.AcrossExtent)) * (Stride + 1u), 0u);
    for (std::uint32_t Across = 0u; Across < Extent.AcrossExtent && Extent.AcrossExtent > 0u; ++Across)
        if (Stride > 0u)
            std::memcpy(&Scanlines[static_cast<std::size_t>(Across) * (Stride + 1u) + 1u],
                        &Extent.Ordinates[static_cast<std::size_t>(Across) * Stride], Stride);

    // ③ zlib stream of stored blocks — honest, uncompressed, dependency-free.
    const std::size_t PayloadExtent = Scanlines.size();
    std::vector<std::uint8_t> Streamed;
    Streamed.reserve(PayloadExtent + PayloadExtent / 65535u * 5u + 16u);
    Streamed.push_back(0x78u);   // [-] - CMF: deflate, 32K window
    Streamed.push_back(0x01u);   // [-] - FLG: no dictionary, fastest
    std::size_t Cursor = 0u;
    while (Cursor < PayloadExtent)
    {
        const std::size_t Block = PayloadExtent - Cursor > 65535u ? 65535u : PayloadExtent - Cursor;
        const bool Terminal = Cursor + Block >= PayloadExtent;
        Streamed.push_back(Terminal ? 1u : 0u);
        Streamed.push_back(static_cast<std::uint8_t>(Block & 0xFFu));
        Streamed.push_back(static_cast<std::uint8_t>(Block >> 8));
        Streamed.push_back(static_cast<std::uint8_t>(~Block & 0xFFu));
        Streamed.push_back(static_cast<std::uint8_t>(~Block >> 8));
        Streamed.insert(Streamed.end(), Scanlines.begin() + static_cast<std::ptrdiff_t>(Cursor),
                        Scanlines.begin() + static_cast<std::ptrdiff_t>(Cursor + Block));
        Cursor += Block;
    }
    const std::uint32_t Adler = AdlerThirtyTwo(Scanlines.data(), Scanlines.size());
    Streamed.push_back(static_cast<std::uint8_t>(Adler >> 24));  Streamed.push_back(static_cast<std::uint8_t>(Adler >> 16));
    Streamed.push_back(static_cast<std::uint8_t>(Adler >> 8));   Streamed.push_back(static_cast<std::uint8_t>(Adler));

    // ④ The chunks — signature, header, payload, end.
    const auto PresentChunk = [&](const std::uint8_t Tag[4], const std::vector<std::uint8_t>& Body)
    {
        WriteBigEndian(Stream, static_cast<std::uint32_t>(Body.size()));
        std::fwrite(Tag, 1u, 4u, Stream);
        std::fwrite(Body.data(), 1u, Body.size(), Stream);
        std::uint32_t Check = CyclicRedundancyCheck(Tag, 4u);
        // ①① CRC runs over tag then body; the table call per byte keeps it simple and honest.
        std::vector<std::uint8_t> Joined(Tag, Tag + 4u);
        Joined.insert(Joined.end(), Body.begin(), Body.end());
        Check = CyclicRedundancyCheck(Joined.data(), Joined.size());
        WriteBigEndian(Stream, Check);
    };

    const std::uint8_t Signature[8] = { 0x89u, 'P', 'N', 'G', '\r', '\n', 0x1Au, '\n' };
    std::fwrite(Signature, 1u, 8u, Stream);

    std::vector<std::uint8_t> Header;
    Header.insert(Header.end(), { static_cast<std::uint8_t>(Extent.AlongExtent >> 24), static_cast<std::uint8_t>(Extent.AlongExtent >> 16),
                                  static_cast<std::uint8_t>(Extent.AlongExtent >> 8), static_cast<std::uint8_t>(Extent.AlongExtent),
                                  static_cast<std::uint8_t>(Extent.AcrossExtent >> 24), static_cast<std::uint8_t>(Extent.AcrossExtent >> 16),
                                  static_cast<std::uint8_t>(Extent.AcrossExtent >> 8), static_cast<std::uint8_t>(Extent.AcrossExtent),
                                  8u, 6u, 0u, 0u, 0u });
    PresentChunk(reinterpret_cast<const std::uint8_t*>("IHDR"), Header);
    PresentChunk(reinterpret_cast<const std::uint8_t*>("IDAT"), Streamed);
    PresentChunk(reinterpret_cast<const std::uint8_t*>("IEND"), {});

    std::fclose(Stream);
    return Deliver<bool>::Delivered(true);
}

}   // namespace Reference
}   // namespace Slate
