#pragma once

#include "core/forward_decls.h"

using UIString = class QString;
namespace StringUtils {
    [[nodiscard]] String to_utf8(const UIString &s);
}
// Tool translate (TTR and variants) for the editor UI,
// and doc translate for the class reference (DTR).
#ifdef TOOLS_ENABLED

// Gets parsed.
StringName TTR(StringView);
StringName DTR(StringView);
#define TTR_utf8(v) StringUtils::to_utf8(TTR(v))

// Use for C strings.
#define TTRC(m_value) (m_value)
// Use to avoid parsing (for use later with C strings).
#define TTRGET(m_value) TTR(m_value)

#else

#define TTR(m_value) (m_value)
#define DTR(m_value) (String())
#define TTRC(m_value) (m_value)
#define TTRGET(m_value) (m_value)

#endif

// Runtime translate for the public node API.
StringName RTR(const char *);
String RTR_utf8(StringView sv);
