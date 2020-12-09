import * as $protobuf from "protobufjs";
/** Namespace squadov. */
export namespace squadov {

    /** Namespace hearthstone. */
    namespace hearthstone {

        /** Namespace game_state. */
        namespace game_state {

            /** Properties of a HearthstoneSerializedGameSnapshotAuxData. */
            interface IHearthstoneSerializedGameSnapshotAuxData {

                /** HearthstoneSerializedGameSnapshotAuxData currentTurn */
                currentTurn?: (number|null);

                /** HearthstoneSerializedGameSnapshotAuxData step */
                step?: (number|null);

                /** HearthstoneSerializedGameSnapshotAuxData currentPlayerId */
                currentPlayerId?: (number|null);

                /** HearthstoneSerializedGameSnapshotAuxData lastActionIndex */
                lastActionIndex?: (number|Long|null);
            }

            /** Represents a HearthstoneSerializedGameSnapshotAuxData. */
            class HearthstoneSerializedGameSnapshotAuxData implements IHearthstoneSerializedGameSnapshotAuxData {

                /**
                 * Constructs a new HearthstoneSerializedGameSnapshotAuxData.
                 * @param [properties] Properties to set
                 */
                constructor(properties?: squadov.hearthstone.game_state.IHearthstoneSerializedGameSnapshotAuxData);

                /** HearthstoneSerializedGameSnapshotAuxData currentTurn. */
                public currentTurn: number;

                /** HearthstoneSerializedGameSnapshotAuxData step. */
                public step: number;

                /** HearthstoneSerializedGameSnapshotAuxData currentPlayerId. */
                public currentPlayerId: number;

                /** HearthstoneSerializedGameSnapshotAuxData lastActionIndex. */
                public lastActionIndex: (number|Long);

                /**
                 * Creates a new HearthstoneSerializedGameSnapshotAuxData instance using the specified properties.
                 * @param [properties] Properties to set
                 * @returns HearthstoneSerializedGameSnapshotAuxData instance
                 */
                public static create(properties?: squadov.hearthstone.game_state.IHearthstoneSerializedGameSnapshotAuxData): squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshotAuxData;

                /**
                 * Encodes the specified HearthstoneSerializedGameSnapshotAuxData message. Does not implicitly {@link squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshotAuxData.verify|verify} messages.
                 * @param message HearthstoneSerializedGameSnapshotAuxData message or plain object to encode
                 * @param [writer] Writer to encode to
                 * @returns Writer
                 */
                public static encode(message: squadov.hearthstone.game_state.IHearthstoneSerializedGameSnapshotAuxData, writer?: $protobuf.Writer): $protobuf.Writer;

                /**
                 * Encodes the specified HearthstoneSerializedGameSnapshotAuxData message, length delimited. Does not implicitly {@link squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshotAuxData.verify|verify} messages.
                 * @param message HearthstoneSerializedGameSnapshotAuxData message or plain object to encode
                 * @param [writer] Writer to encode to
                 * @returns Writer
                 */
                public static encodeDelimited(message: squadov.hearthstone.game_state.IHearthstoneSerializedGameSnapshotAuxData, writer?: $protobuf.Writer): $protobuf.Writer;

                /**
                 * Decodes a HearthstoneSerializedGameSnapshotAuxData message from the specified reader or buffer.
                 * @param reader Reader or buffer to decode from
                 * @param [length] Message length if known beforehand
                 * @returns HearthstoneSerializedGameSnapshotAuxData
                 * @throws {Error} If the payload is not a reader or valid buffer
                 * @throws {$protobuf.util.ProtocolError} If required fields are missing
                 */
                public static decode(reader: ($protobuf.Reader|Uint8Array), length?: number): squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshotAuxData;

                /**
                 * Decodes a HearthstoneSerializedGameSnapshotAuxData message from the specified reader or buffer, length delimited.
                 * @param reader Reader or buffer to decode from
                 * @returns HearthstoneSerializedGameSnapshotAuxData
                 * @throws {Error} If the payload is not a reader or valid buffer
                 * @throws {$protobuf.util.ProtocolError} If required fields are missing
                 */
                public static decodeDelimited(reader: ($protobuf.Reader|Uint8Array)): squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshotAuxData;

                /**
                 * Verifies a HearthstoneSerializedGameSnapshotAuxData message.
                 * @param message Plain object to verify
                 * @returns `null` if valid, otherwise the reason why it is not
                 */
                public static verify(message: { [k: string]: any }): (string|null);

                /**
                 * Creates a HearthstoneSerializedGameSnapshotAuxData message from a plain object. Also converts values to their respective internal types.
                 * @param object Plain object
                 * @returns HearthstoneSerializedGameSnapshotAuxData
                 */
                public static fromObject(object: { [k: string]: any }): squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshotAuxData;

                /**
                 * Creates a plain object from a HearthstoneSerializedGameSnapshotAuxData message. Also converts values to other types if specified.
                 * @param message HearthstoneSerializedGameSnapshotAuxData
                 * @param [options] Conversion options
                 * @returns Plain object
                 */
                public static toObject(message: squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshotAuxData, options?: $protobuf.IConversionOptions): { [k: string]: any };

                /**
                 * Converts this HearthstoneSerializedGameSnapshotAuxData to JSON.
                 * @returns JSON object
                 */
                public toJSON(): { [k: string]: any };
            }

            /** Properties of a HearthstoneSerializedEntity. */
            interface IHearthstoneSerializedEntity {

                /** HearthstoneSerializedEntity entityId */
                entityId?: (number|null);

                /** HearthstoneSerializedEntity tags */
                tags?: (string|null);

                /** HearthstoneSerializedEntity attributes */
                attributes?: (string|null);
            }

            /** Represents a HearthstoneSerializedEntity. */
            class HearthstoneSerializedEntity implements IHearthstoneSerializedEntity {

                /**
                 * Constructs a new HearthstoneSerializedEntity.
                 * @param [properties] Properties to set
                 */
                constructor(properties?: squadov.hearthstone.game_state.IHearthstoneSerializedEntity);

                /** HearthstoneSerializedEntity entityId. */
                public entityId: number;

                /** HearthstoneSerializedEntity tags. */
                public tags: string;

                /** HearthstoneSerializedEntity attributes. */
                public attributes: string;

                /**
                 * Creates a new HearthstoneSerializedEntity instance using the specified properties.
                 * @param [properties] Properties to set
                 * @returns HearthstoneSerializedEntity instance
                 */
                public static create(properties?: squadov.hearthstone.game_state.IHearthstoneSerializedEntity): squadov.hearthstone.game_state.HearthstoneSerializedEntity;

                /**
                 * Encodes the specified HearthstoneSerializedEntity message. Does not implicitly {@link squadov.hearthstone.game_state.HearthstoneSerializedEntity.verify|verify} messages.
                 * @param message HearthstoneSerializedEntity message or plain object to encode
                 * @param [writer] Writer to encode to
                 * @returns Writer
                 */
                public static encode(message: squadov.hearthstone.game_state.IHearthstoneSerializedEntity, writer?: $protobuf.Writer): $protobuf.Writer;

                /**
                 * Encodes the specified HearthstoneSerializedEntity message, length delimited. Does not implicitly {@link squadov.hearthstone.game_state.HearthstoneSerializedEntity.verify|verify} messages.
                 * @param message HearthstoneSerializedEntity message or plain object to encode
                 * @param [writer] Writer to encode to
                 * @returns Writer
                 */
                public static encodeDelimited(message: squadov.hearthstone.game_state.IHearthstoneSerializedEntity, writer?: $protobuf.Writer): $protobuf.Writer;

                /**
                 * Decodes a HearthstoneSerializedEntity message from the specified reader or buffer.
                 * @param reader Reader or buffer to decode from
                 * @param [length] Message length if known beforehand
                 * @returns HearthstoneSerializedEntity
                 * @throws {Error} If the payload is not a reader or valid buffer
                 * @throws {$protobuf.util.ProtocolError} If required fields are missing
                 */
                public static decode(reader: ($protobuf.Reader|Uint8Array), length?: number): squadov.hearthstone.game_state.HearthstoneSerializedEntity;

                /**
                 * Decodes a HearthstoneSerializedEntity message from the specified reader or buffer, length delimited.
                 * @param reader Reader or buffer to decode from
                 * @returns HearthstoneSerializedEntity
                 * @throws {Error} If the payload is not a reader or valid buffer
                 * @throws {$protobuf.util.ProtocolError} If required fields are missing
                 */
                public static decodeDelimited(reader: ($protobuf.Reader|Uint8Array)): squadov.hearthstone.game_state.HearthstoneSerializedEntity;

                /**
                 * Verifies a HearthstoneSerializedEntity message.
                 * @param message Plain object to verify
                 * @returns `null` if valid, otherwise the reason why it is not
                 */
                public static verify(message: { [k: string]: any }): (string|null);

                /**
                 * Creates a HearthstoneSerializedEntity message from a plain object. Also converts values to their respective internal types.
                 * @param object Plain object
                 * @returns HearthstoneSerializedEntity
                 */
                public static fromObject(object: { [k: string]: any }): squadov.hearthstone.game_state.HearthstoneSerializedEntity;

                /**
                 * Creates a plain object from a HearthstoneSerializedEntity message. Also converts values to other types if specified.
                 * @param message HearthstoneSerializedEntity
                 * @param [options] Conversion options
                 * @returns Plain object
                 */
                public static toObject(message: squadov.hearthstone.game_state.HearthstoneSerializedEntity, options?: $protobuf.IConversionOptions): { [k: string]: any };

                /**
                 * Converts this HearthstoneSerializedEntity to JSON.
                 * @returns JSON object
                 */
                public toJSON(): { [k: string]: any };
            }

            /** Properties of a HearthstoneSerializedGameSnapshot. */
            interface IHearthstoneSerializedGameSnapshot {

                /** HearthstoneSerializedGameSnapshot uuid */
                uuid?: (string|null);

                /** HearthstoneSerializedGameSnapshot tm */
                tm?: (number|Long|null);

                /** HearthstoneSerializedGameSnapshot gameEntityId */
                gameEntityId?: (number|null);

                /** HearthstoneSerializedGameSnapshot playerNameToPlayerId */
                playerNameToPlayerId?: ({ [k: string]: number }|null);

                /** HearthstoneSerializedGameSnapshot playerIdToEntityId */
                playerIdToEntityId?: ({ [k: string]: number }|null);

                /** HearthstoneSerializedGameSnapshot entities */
                entities?: ({ [k: string]: squadov.hearthstone.game_state.IHearthstoneSerializedEntity }|null);

                /** HearthstoneSerializedGameSnapshot auxData */
                auxData?: (squadov.hearthstone.game_state.IHearthstoneSerializedGameSnapshotAuxData|null);
            }

            /** Represents a HearthstoneSerializedGameSnapshot. */
            class HearthstoneSerializedGameSnapshot implements IHearthstoneSerializedGameSnapshot {

                /**
                 * Constructs a new HearthstoneSerializedGameSnapshot.
                 * @param [properties] Properties to set
                 */
                constructor(properties?: squadov.hearthstone.game_state.IHearthstoneSerializedGameSnapshot);

                /** HearthstoneSerializedGameSnapshot uuid. */
                public uuid: string;

                /** HearthstoneSerializedGameSnapshot tm. */
                public tm: (number|Long);

                /** HearthstoneSerializedGameSnapshot gameEntityId. */
                public gameEntityId: number;

                /** HearthstoneSerializedGameSnapshot playerNameToPlayerId. */
                public playerNameToPlayerId: { [k: string]: number };

                /** HearthstoneSerializedGameSnapshot playerIdToEntityId. */
                public playerIdToEntityId: { [k: string]: number };

                /** HearthstoneSerializedGameSnapshot entities. */
                public entities: { [k: string]: squadov.hearthstone.game_state.IHearthstoneSerializedEntity };

                /** HearthstoneSerializedGameSnapshot auxData. */
                public auxData?: (squadov.hearthstone.game_state.IHearthstoneSerializedGameSnapshotAuxData|null);

                /**
                 * Creates a new HearthstoneSerializedGameSnapshot instance using the specified properties.
                 * @param [properties] Properties to set
                 * @returns HearthstoneSerializedGameSnapshot instance
                 */
                public static create(properties?: squadov.hearthstone.game_state.IHearthstoneSerializedGameSnapshot): squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshot;

                /**
                 * Encodes the specified HearthstoneSerializedGameSnapshot message. Does not implicitly {@link squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshot.verify|verify} messages.
                 * @param message HearthstoneSerializedGameSnapshot message or plain object to encode
                 * @param [writer] Writer to encode to
                 * @returns Writer
                 */
                public static encode(message: squadov.hearthstone.game_state.IHearthstoneSerializedGameSnapshot, writer?: $protobuf.Writer): $protobuf.Writer;

                /**
                 * Encodes the specified HearthstoneSerializedGameSnapshot message, length delimited. Does not implicitly {@link squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshot.verify|verify} messages.
                 * @param message HearthstoneSerializedGameSnapshot message or plain object to encode
                 * @param [writer] Writer to encode to
                 * @returns Writer
                 */
                public static encodeDelimited(message: squadov.hearthstone.game_state.IHearthstoneSerializedGameSnapshot, writer?: $protobuf.Writer): $protobuf.Writer;

                /**
                 * Decodes a HearthstoneSerializedGameSnapshot message from the specified reader or buffer.
                 * @param reader Reader or buffer to decode from
                 * @param [length] Message length if known beforehand
                 * @returns HearthstoneSerializedGameSnapshot
                 * @throws {Error} If the payload is not a reader or valid buffer
                 * @throws {$protobuf.util.ProtocolError} If required fields are missing
                 */
                public static decode(reader: ($protobuf.Reader|Uint8Array), length?: number): squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshot;

                /**
                 * Decodes a HearthstoneSerializedGameSnapshot message from the specified reader or buffer, length delimited.
                 * @param reader Reader or buffer to decode from
                 * @returns HearthstoneSerializedGameSnapshot
                 * @throws {Error} If the payload is not a reader or valid buffer
                 * @throws {$protobuf.util.ProtocolError} If required fields are missing
                 */
                public static decodeDelimited(reader: ($protobuf.Reader|Uint8Array)): squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshot;

                /**
                 * Verifies a HearthstoneSerializedGameSnapshot message.
                 * @param message Plain object to verify
                 * @returns `null` if valid, otherwise the reason why it is not
                 */
                public static verify(message: { [k: string]: any }): (string|null);

                /**
                 * Creates a HearthstoneSerializedGameSnapshot message from a plain object. Also converts values to their respective internal types.
                 * @param object Plain object
                 * @returns HearthstoneSerializedGameSnapshot
                 */
                public static fromObject(object: { [k: string]: any }): squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshot;

                /**
                 * Creates a plain object from a HearthstoneSerializedGameSnapshot message. Also converts values to other types if specified.
                 * @param message HearthstoneSerializedGameSnapshot
                 * @param [options] Conversion options
                 * @returns Plain object
                 */
                public static toObject(message: squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshot, options?: $protobuf.IConversionOptions): { [k: string]: any };

                /**
                 * Converts this HearthstoneSerializedGameSnapshot to JSON.
                 * @returns JSON object
                 */
                public toJSON(): { [k: string]: any };
            }

            /** Properties of a HearthstoneSerializedGameBlock. */
            interface IHearthstoneSerializedGameBlock {

                /** HearthstoneSerializedGameBlock blockId */
                blockId?: (string|null);

                /** HearthstoneSerializedGameBlock startActionIndex */
                startActionIndex?: (number|null);

                /** HearthstoneSerializedGameBlock endActionIndex */
                endActionIndex?: (number|null);

                /** HearthstoneSerializedGameBlock blockType */
                blockType?: (number|null);

                /** HearthstoneSerializedGameBlock parentBlock */
                parentBlock?: (string|null);

                /** HearthstoneSerializedGameBlock entityId */
                entityId?: (number|null);
            }

            /** Represents a HearthstoneSerializedGameBlock. */
            class HearthstoneSerializedGameBlock implements IHearthstoneSerializedGameBlock {

                /**
                 * Constructs a new HearthstoneSerializedGameBlock.
                 * @param [properties] Properties to set
                 */
                constructor(properties?: squadov.hearthstone.game_state.IHearthstoneSerializedGameBlock);

                /** HearthstoneSerializedGameBlock blockId. */
                public blockId: string;

                /** HearthstoneSerializedGameBlock startActionIndex. */
                public startActionIndex: number;

                /** HearthstoneSerializedGameBlock endActionIndex. */
                public endActionIndex: number;

                /** HearthstoneSerializedGameBlock blockType. */
                public blockType: number;

                /** HearthstoneSerializedGameBlock parentBlock. */
                public parentBlock: string;

                /** HearthstoneSerializedGameBlock entityId. */
                public entityId: number;

                /**
                 * Creates a new HearthstoneSerializedGameBlock instance using the specified properties.
                 * @param [properties] Properties to set
                 * @returns HearthstoneSerializedGameBlock instance
                 */
                public static create(properties?: squadov.hearthstone.game_state.IHearthstoneSerializedGameBlock): squadov.hearthstone.game_state.HearthstoneSerializedGameBlock;

                /**
                 * Encodes the specified HearthstoneSerializedGameBlock message. Does not implicitly {@link squadov.hearthstone.game_state.HearthstoneSerializedGameBlock.verify|verify} messages.
                 * @param message HearthstoneSerializedGameBlock message or plain object to encode
                 * @param [writer] Writer to encode to
                 * @returns Writer
                 */
                public static encode(message: squadov.hearthstone.game_state.IHearthstoneSerializedGameBlock, writer?: $protobuf.Writer): $protobuf.Writer;

                /**
                 * Encodes the specified HearthstoneSerializedGameBlock message, length delimited. Does not implicitly {@link squadov.hearthstone.game_state.HearthstoneSerializedGameBlock.verify|verify} messages.
                 * @param message HearthstoneSerializedGameBlock message or plain object to encode
                 * @param [writer] Writer to encode to
                 * @returns Writer
                 */
                public static encodeDelimited(message: squadov.hearthstone.game_state.IHearthstoneSerializedGameBlock, writer?: $protobuf.Writer): $protobuf.Writer;

                /**
                 * Decodes a HearthstoneSerializedGameBlock message from the specified reader or buffer.
                 * @param reader Reader or buffer to decode from
                 * @param [length] Message length if known beforehand
                 * @returns HearthstoneSerializedGameBlock
                 * @throws {Error} If the payload is not a reader or valid buffer
                 * @throws {$protobuf.util.ProtocolError} If required fields are missing
                 */
                public static decode(reader: ($protobuf.Reader|Uint8Array), length?: number): squadov.hearthstone.game_state.HearthstoneSerializedGameBlock;

                /**
                 * Decodes a HearthstoneSerializedGameBlock message from the specified reader or buffer, length delimited.
                 * @param reader Reader or buffer to decode from
                 * @returns HearthstoneSerializedGameBlock
                 * @throws {Error} If the payload is not a reader or valid buffer
                 * @throws {$protobuf.util.ProtocolError} If required fields are missing
                 */
                public static decodeDelimited(reader: ($protobuf.Reader|Uint8Array)): squadov.hearthstone.game_state.HearthstoneSerializedGameBlock;

                /**
                 * Verifies a HearthstoneSerializedGameBlock message.
                 * @param message Plain object to verify
                 * @returns `null` if valid, otherwise the reason why it is not
                 */
                public static verify(message: { [k: string]: any }): (string|null);

                /**
                 * Creates a HearthstoneSerializedGameBlock message from a plain object. Also converts values to their respective internal types.
                 * @param object Plain object
                 * @returns HearthstoneSerializedGameBlock
                 */
                public static fromObject(object: { [k: string]: any }): squadov.hearthstone.game_state.HearthstoneSerializedGameBlock;

                /**
                 * Creates a plain object from a HearthstoneSerializedGameBlock message. Also converts values to other types if specified.
                 * @param message HearthstoneSerializedGameBlock
                 * @param [options] Conversion options
                 * @returns Plain object
                 */
                public static toObject(message: squadov.hearthstone.game_state.HearthstoneSerializedGameBlock, options?: $protobuf.IConversionOptions): { [k: string]: any };

                /**
                 * Converts this HearthstoneSerializedGameBlock to JSON.
                 * @returns JSON object
                 */
                public toJSON(): { [k: string]: any };
            }

            /** Properties of a HearthstoneSerializedGameLog. */
            interface IHearthstoneSerializedGameLog {

                /** HearthstoneSerializedGameLog snapshots */
                snapshots?: (squadov.hearthstone.game_state.IHearthstoneSerializedGameSnapshot[]|null);

                /** HearthstoneSerializedGameLog actions */
                actions?: (string|null);

                /** HearthstoneSerializedGameLog blocks */
                blocks?: (squadov.hearthstone.game_state.IHearthstoneSerializedGameBlock[]|null);
            }

            /** Represents a HearthstoneSerializedGameLog. */
            class HearthstoneSerializedGameLog implements IHearthstoneSerializedGameLog {

                /**
                 * Constructs a new HearthstoneSerializedGameLog.
                 * @param [properties] Properties to set
                 */
                constructor(properties?: squadov.hearthstone.game_state.IHearthstoneSerializedGameLog);

                /** HearthstoneSerializedGameLog snapshots. */
                public snapshots: squadov.hearthstone.game_state.IHearthstoneSerializedGameSnapshot[];

                /** HearthstoneSerializedGameLog actions. */
                public actions: string;

                /** HearthstoneSerializedGameLog blocks. */
                public blocks: squadov.hearthstone.game_state.IHearthstoneSerializedGameBlock[];

                /**
                 * Creates a new HearthstoneSerializedGameLog instance using the specified properties.
                 * @param [properties] Properties to set
                 * @returns HearthstoneSerializedGameLog instance
                 */
                public static create(properties?: squadov.hearthstone.game_state.IHearthstoneSerializedGameLog): squadov.hearthstone.game_state.HearthstoneSerializedGameLog;

                /**
                 * Encodes the specified HearthstoneSerializedGameLog message. Does not implicitly {@link squadov.hearthstone.game_state.HearthstoneSerializedGameLog.verify|verify} messages.
                 * @param message HearthstoneSerializedGameLog message or plain object to encode
                 * @param [writer] Writer to encode to
                 * @returns Writer
                 */
                public static encode(message: squadov.hearthstone.game_state.IHearthstoneSerializedGameLog, writer?: $protobuf.Writer): $protobuf.Writer;

                /**
                 * Encodes the specified HearthstoneSerializedGameLog message, length delimited. Does not implicitly {@link squadov.hearthstone.game_state.HearthstoneSerializedGameLog.verify|verify} messages.
                 * @param message HearthstoneSerializedGameLog message or plain object to encode
                 * @param [writer] Writer to encode to
                 * @returns Writer
                 */
                public static encodeDelimited(message: squadov.hearthstone.game_state.IHearthstoneSerializedGameLog, writer?: $protobuf.Writer): $protobuf.Writer;

                /**
                 * Decodes a HearthstoneSerializedGameLog message from the specified reader or buffer.
                 * @param reader Reader or buffer to decode from
                 * @param [length] Message length if known beforehand
                 * @returns HearthstoneSerializedGameLog
                 * @throws {Error} If the payload is not a reader or valid buffer
                 * @throws {$protobuf.util.ProtocolError} If required fields are missing
                 */
                public static decode(reader: ($protobuf.Reader|Uint8Array), length?: number): squadov.hearthstone.game_state.HearthstoneSerializedGameLog;

                /**
                 * Decodes a HearthstoneSerializedGameLog message from the specified reader or buffer, length delimited.
                 * @param reader Reader or buffer to decode from
                 * @returns HearthstoneSerializedGameLog
                 * @throws {Error} If the payload is not a reader or valid buffer
                 * @throws {$protobuf.util.ProtocolError} If required fields are missing
                 */
                public static decodeDelimited(reader: ($protobuf.Reader|Uint8Array)): squadov.hearthstone.game_state.HearthstoneSerializedGameLog;

                /**
                 * Verifies a HearthstoneSerializedGameLog message.
                 * @param message Plain object to verify
                 * @returns `null` if valid, otherwise the reason why it is not
                 */
                public static verify(message: { [k: string]: any }): (string|null);

                /**
                 * Creates a HearthstoneSerializedGameLog message from a plain object. Also converts values to their respective internal types.
                 * @param object Plain object
                 * @returns HearthstoneSerializedGameLog
                 */
                public static fromObject(object: { [k: string]: any }): squadov.hearthstone.game_state.HearthstoneSerializedGameLog;

                /**
                 * Creates a plain object from a HearthstoneSerializedGameLog message. Also converts values to other types if specified.
                 * @param message HearthstoneSerializedGameLog
                 * @param [options] Conversion options
                 * @returns Plain object
                 */
                public static toObject(message: squadov.hearthstone.game_state.HearthstoneSerializedGameLog, options?: $protobuf.IConversionOptions): { [k: string]: any };

                /**
                 * Converts this HearthstoneSerializedGameLog to JSON.
                 * @returns JSON object
                 */
                public toJSON(): { [k: string]: any };
            }
        }
    }
}
