#include <reader/analyzer.h>
#include <format>

bool Analyzer::analyzeTag(TaggedBlockReader *reader, const bool skipGettingTag, const bool skipClaim) {
    if (!skipGettingTag) {
        if (!skipClaim && !reader->tagClaimed) {
            reader->claimTag(); // Ensure we are reading the next tag if the current one was unclaimed
        }
        reader->getTag();
    }
    if (reader->tag.type == TagType::BAD_LENGTH || reader->tag.type == TagType::BAD_VALUINT) {
        logDebug(std::format(
            "{}BAD Read [TAG] index: {}, type: 0x{:X} <{}> offset: {}",
            paddingStr,
            reader->tag.index,
            static_cast<unsigned int>(reader->tag.type),
            debugTagTypes(reader->tag.type),
            reader->currentOffset
        ));
        return false;
    }
    logDebug(std::format(
        "{}Read [TAG] index: {}, type: 0x{:X} <{}> offset: {}",
        paddingStr,
        reader->tag.index,
        static_cast<unsigned int>(reader->tag.type),
        debugTagTypes(reader->tag.type),
        reader->currentOffset
    ));
    return true;
}

void Analyzer::continuousRead(TaggedBlockReader *reader) {
    logDebug(std::format("Starting offset: {}", reader->currentOffset));

    while (reader->hasBytesRemaining()) {
        if (!reader->tagClaimed) {
            reader->claimTag();
        }
        reader->getTag();
        if (reader->tag.type != TagType::BAD_LENGTH && reader->tag.type != TagType::BAD_VALUINT) {
            logDebug(std::format(
                "Read [TAG] index: {}, type: 0x{:X} <{}> offset: {}",
                reader->tag.index,
                static_cast<unsigned int>(reader->tag.type),
                debugTagTypes(reader->tag.type),
                reader->currentOffset
            ));
            if (reader->tag.type == TagType::MisreadValuint) {
                uint64_t value;
                reader->readValuint(value);
            }
        }
    }
}

void Analyzer::analyzeData(TaggedBlockReader *reader) {
    setPadding(0);
    int i = 0;
    while (reader->hasBytesRemaining()) {
        bool tagFoundInValuint = false;
        if (auto info = getSubBlockInfo(); info.has_value()) {
            auto revertOffset = reader->currentOffset;
            std::string final = std::format("{}<< Potential [Valuint(s)]", paddingStr);
            while (reader->currentOffset < info->offset + info->size) {
                if (i > 0 && !reader->tagClaimed) {
                    reader->claimTag();
                }
                reader->getTag();
                if (reader->tag.type != TagType::BAD_LENGTH && reader->tag.type != TagType::BAD_VALUINT) {
                    // Potentially good tag, abort the valuint(s)
                    revertOffset = reader->currentOffset; // Update the revert offset to the last known good tag
                    tagFoundInValuint = true;
                    break;
                }
                if (uint64_t valInt; reader->readValuint(valInt)) {
                    final += std::format(" {}", valInt);
                } else {
                    break;
                }
            }
            if (!tagFoundInValuint) {
                logDebug(final);
            }
            reader->seekTo(revertOffset); // Revert back to the original offset after the peek
        }

        if (!analyzeTag(reader, tagFoundInValuint, i == 0)) {
            break;
        }
        i++;
        if (getCurrentSubBlockIndex() > -1 && reader->tag.index > getCurrentSubBlockIndex()) {
            decPadding();
            subBlockInfos.pop();
            subBlockIndexes.pop();
        }
        switch (reader->tag.type) {
            case TagType::BAD_LENGTH:
            case TagType::BAD_VALUINT:
                // If we encounter a bad tag, we should stop analyzing further to avoid infinite loops or misinterpretation of data.
                logError(std::format("{}Encountered bad tag type: 0x{:X} <{}>. Stopping analysis.",
                                     paddingStr, static_cast<unsigned int>(reader->tag.type),
                                     debugTagTypes(reader->tag.type)));
                return;
            case TagType::ID: {
                CrdtId id;
                reader->readId(&id);
                logDebug(std::format("{}Read [ID] {}", paddingStrSub, id.repr()));
                break;
            }
            case TagType::Byte1: {
                // Likely a bool
                uint8_t value;
                reader->readByte(&value);
                logDebug(std::format("{}Read [Byte1] {} 0x{:X}", paddingStrSub, value, value));
                logDebug(std::format("{} -- as bool: {}", paddingStrSub, value != 0));
                break;
            }
            case TagType::MisreadValuint: {
                uint64_t value;
                reader->readValuint(value);
                logDebug(std::format("{}Read [Valuint] {}", paddingStrSub, value));
                break;
            }
            case TagType::Byte4: {
                // Likely a 32 bit int
                uint32_t value;
                reader->readInt(&value);

                const float value_f = *reinterpret_cast<float *>(&value);
                const Color value_c = Color::fromARGB(&value);

                logDebug(std::format("{}Read [Byte4]", paddingStrSub));
                logDebug(std::format("{} -- as int: {}", paddingStrSub, value));
                logDebug(std::format("{} -- as float: {}", paddingStrSub, value_f));
                logDebug(std::format("{} -- as color: {}", paddingStrSub, value_c.repr()));
                logDebug(std::format("{} -- as bytes: {}", paddingStrSub, bytesToHexStr({
                                         static_cast<uint8_t>(value & 0xFF), static_cast<uint8_t>((value >> 8) & 0xFF),
                                         static_cast<uint8_t>((value >> 16) & 0xFF),
                                         static_cast<uint8_t>((value >> 24) & 0xFF)
                                     })));
                break;
            }
            case TagType::Length4: {
                SubBlockInfo info;
                if (!reader->readSubBlock(reader->tag.index, info)) {
                    logError(std::format(
                        "{}Failed to read subblock for tag index {}. Current offset: {}, data size: {}",
                        paddingStr, reader->tag.index, reader->currentOffset, reader->dataSize_));
                    return;
                }
                // This sub block might be a part of a string, try to read it
                std::string _result;
                int rollbackOffset = reader->currentOffset;
                if (reader->readString(&_result)) {
                    // Verify this data is meaningful, for example a uuid or text
                    if (_result.length() >= 4 && isValidString(_result)) {
                        logDebug(std::format("{}Read [String] \"{}\"", paddingStrSub, _result));
                        break;
                    }
                }
                reader->seekTo(rollbackOffset); // Rollback if it wasn't a valid string

                logDebug(std::format(
                    "{}SubBlockInfo - offset: {}, size: {}",
                    paddingStrSub, info.offset, info.size
                ));

                incPadding();
                subBlockInfos.push(info);
                subBlockIndexes.push(reader->tag.index);
                break;
            }
            default:
                break;
        }
    }
}

void Analyzer::analyze(TaggedBlockReader *reader) {
    logDebug(std::format("-> Analyzing block type {} (0x{:X}) at offset {} with size {}",
                         reader->currentBlockInfo.blockType,
                         reader->currentBlockInfo.blockType,
                         reader->currentBlockInfo.offset,
                         reader->currentBlockInfo.size));
    auto analyzer = Analyzer();
    analyzer.analyzeData(reader);
    if (!reader->hasBytesRemaining()) {
        logDebug("-> CONCLUDE <------- FULL READ");
    } else {
        int remaining = reader->remainingBytes();
        logDebug(std::format(" -> CONCLUDE <------- PARTIAL READ: {}/{} [{}] bytes remain",
                             reader->currentBlockInfo.size - remaining,
                             reader->currentBlockInfo.size, remaining));
        analyzer.continuousRead(reader);
    }
}
