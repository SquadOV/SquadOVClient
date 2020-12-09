/*eslint-disable block-scoped-var, id-length, no-control-regex, no-magic-numbers, no-prototype-builtins, no-redeclare, no-shadow, no-var, sort-vars*/
(function(global, factory) { /* global define, require, module */

    /* AMD */ if (typeof define === 'function' && define.amd)
        define(["protobufjs/minimal"], factory);

    /* CommonJS */ else if (typeof require === 'function' && typeof module === 'object' && module && module.exports)
        module.exports = factory(require("protobufjs/minimal"));

})(this, function($protobuf) {
    "use strict";

    // Common aliases
    var $Reader = $protobuf.Reader, $Writer = $protobuf.Writer, $util = $protobuf.util;
    
    // Exported root namespace
    var $root = $protobuf.roots["default"] || ($protobuf.roots["default"] = {});
    
    $root.squadov = (function() {
    
        /**
         * Namespace squadov.
         * @exports squadov
         * @namespace
         */
        var squadov = {};
    
        squadov.hearthstone = (function() {
    
            /**
             * Namespace hearthstone.
             * @memberof squadov
             * @namespace
             */
            var hearthstone = {};
    
            hearthstone.game_state = (function() {
    
                /**
                 * Namespace game_state.
                 * @memberof squadov.hearthstone
                 * @namespace
                 */
                var game_state = {};
    
                game_state.HearthstoneSerializedGameSnapshotAuxData = (function() {
    
                    /**
                     * Properties of a HearthstoneSerializedGameSnapshotAuxData.
                     * @memberof squadov.hearthstone.game_state
                     * @interface IHearthstoneSerializedGameSnapshotAuxData
                     * @property {number|null} [currentTurn] HearthstoneSerializedGameSnapshotAuxData currentTurn
                     * @property {number|null} [step] HearthstoneSerializedGameSnapshotAuxData step
                     * @property {number|null} [currentPlayerId] HearthstoneSerializedGameSnapshotAuxData currentPlayerId
                     * @property {number|Long|null} [lastActionIndex] HearthstoneSerializedGameSnapshotAuxData lastActionIndex
                     */
    
                    /**
                     * Constructs a new HearthstoneSerializedGameSnapshotAuxData.
                     * @memberof squadov.hearthstone.game_state
                     * @classdesc Represents a HearthstoneSerializedGameSnapshotAuxData.
                     * @implements IHearthstoneSerializedGameSnapshotAuxData
                     * @constructor
                     * @param {squadov.hearthstone.game_state.IHearthstoneSerializedGameSnapshotAuxData=} [properties] Properties to set
                     */
                    function HearthstoneSerializedGameSnapshotAuxData(properties) {
                        if (properties)
                            for (var keys = Object.keys(properties), i = 0; i < keys.length; ++i)
                                if (properties[keys[i]] != null)
                                    this[keys[i]] = properties[keys[i]];
                    }
    
                    /**
                     * HearthstoneSerializedGameSnapshotAuxData currentTurn.
                     * @member {number} currentTurn
                     * @memberof squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshotAuxData
                     * @instance
                     */
                    HearthstoneSerializedGameSnapshotAuxData.prototype.currentTurn = 0;
    
                    /**
                     * HearthstoneSerializedGameSnapshotAuxData step.
                     * @member {number} step
                     * @memberof squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshotAuxData
                     * @instance
                     */
                    HearthstoneSerializedGameSnapshotAuxData.prototype.step = 0;
    
                    /**
                     * HearthstoneSerializedGameSnapshotAuxData currentPlayerId.
                     * @member {number} currentPlayerId
                     * @memberof squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshotAuxData
                     * @instance
                     */
                    HearthstoneSerializedGameSnapshotAuxData.prototype.currentPlayerId = 0;
    
                    /**
                     * HearthstoneSerializedGameSnapshotAuxData lastActionIndex.
                     * @member {number|Long} lastActionIndex
                     * @memberof squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshotAuxData
                     * @instance
                     */
                    HearthstoneSerializedGameSnapshotAuxData.prototype.lastActionIndex = $util.Long ? $util.Long.fromBits(0,0,true) : 0;
    
                    /**
                     * Creates a new HearthstoneSerializedGameSnapshotAuxData instance using the specified properties.
                     * @function create
                     * @memberof squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshotAuxData
                     * @static
                     * @param {squadov.hearthstone.game_state.IHearthstoneSerializedGameSnapshotAuxData=} [properties] Properties to set
                     * @returns {squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshotAuxData} HearthstoneSerializedGameSnapshotAuxData instance
                     */
                    HearthstoneSerializedGameSnapshotAuxData.create = function create(properties) {
                        return new HearthstoneSerializedGameSnapshotAuxData(properties);
                    };
    
                    /**
                     * Encodes the specified HearthstoneSerializedGameSnapshotAuxData message. Does not implicitly {@link squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshotAuxData.verify|verify} messages.
                     * @function encode
                     * @memberof squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshotAuxData
                     * @static
                     * @param {squadov.hearthstone.game_state.IHearthstoneSerializedGameSnapshotAuxData} message HearthstoneSerializedGameSnapshotAuxData message or plain object to encode
                     * @param {$protobuf.Writer} [writer] Writer to encode to
                     * @returns {$protobuf.Writer} Writer
                     */
                    HearthstoneSerializedGameSnapshotAuxData.encode = function encode(message, writer) {
                        if (!writer)
                            writer = $Writer.create();
                        if (message.currentTurn != null && Object.hasOwnProperty.call(message, "currentTurn"))
                            writer.uint32(/* id 1, wireType 0 =*/8).int32(message.currentTurn);
                        if (message.step != null && Object.hasOwnProperty.call(message, "step"))
                            writer.uint32(/* id 2, wireType 0 =*/16).int32(message.step);
                        if (message.currentPlayerId != null && Object.hasOwnProperty.call(message, "currentPlayerId"))
                            writer.uint32(/* id 3, wireType 0 =*/24).int32(message.currentPlayerId);
                        if (message.lastActionIndex != null && Object.hasOwnProperty.call(message, "lastActionIndex"))
                            writer.uint32(/* id 4, wireType 0 =*/32).uint64(message.lastActionIndex);
                        return writer;
                    };
    
                    /**
                     * Encodes the specified HearthstoneSerializedGameSnapshotAuxData message, length delimited. Does not implicitly {@link squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshotAuxData.verify|verify} messages.
                     * @function encodeDelimited
                     * @memberof squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshotAuxData
                     * @static
                     * @param {squadov.hearthstone.game_state.IHearthstoneSerializedGameSnapshotAuxData} message HearthstoneSerializedGameSnapshotAuxData message or plain object to encode
                     * @param {$protobuf.Writer} [writer] Writer to encode to
                     * @returns {$protobuf.Writer} Writer
                     */
                    HearthstoneSerializedGameSnapshotAuxData.encodeDelimited = function encodeDelimited(message, writer) {
                        return this.encode(message, writer).ldelim();
                    };
    
                    /**
                     * Decodes a HearthstoneSerializedGameSnapshotAuxData message from the specified reader or buffer.
                     * @function decode
                     * @memberof squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshotAuxData
                     * @static
                     * @param {$protobuf.Reader|Uint8Array} reader Reader or buffer to decode from
                     * @param {number} [length] Message length if known beforehand
                     * @returns {squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshotAuxData} HearthstoneSerializedGameSnapshotAuxData
                     * @throws {Error} If the payload is not a reader or valid buffer
                     * @throws {$protobuf.util.ProtocolError} If required fields are missing
                     */
                    HearthstoneSerializedGameSnapshotAuxData.decode = function decode(reader, length) {
                        if (!(reader instanceof $Reader))
                            reader = $Reader.create(reader);
                        var end = length === undefined ? reader.len : reader.pos + length, message = new $root.squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshotAuxData();
                        while (reader.pos < end) {
                            var tag = reader.uint32();
                            switch (tag >>> 3) {
                            case 1:
                                message.currentTurn = reader.int32();
                                break;
                            case 2:
                                message.step = reader.int32();
                                break;
                            case 3:
                                message.currentPlayerId = reader.int32();
                                break;
                            case 4:
                                message.lastActionIndex = reader.uint64();
                                break;
                            default:
                                reader.skipType(tag & 7);
                                break;
                            }
                        }
                        return message;
                    };
    
                    /**
                     * Decodes a HearthstoneSerializedGameSnapshotAuxData message from the specified reader or buffer, length delimited.
                     * @function decodeDelimited
                     * @memberof squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshotAuxData
                     * @static
                     * @param {$protobuf.Reader|Uint8Array} reader Reader or buffer to decode from
                     * @returns {squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshotAuxData} HearthstoneSerializedGameSnapshotAuxData
                     * @throws {Error} If the payload is not a reader or valid buffer
                     * @throws {$protobuf.util.ProtocolError} If required fields are missing
                     */
                    HearthstoneSerializedGameSnapshotAuxData.decodeDelimited = function decodeDelimited(reader) {
                        if (!(reader instanceof $Reader))
                            reader = new $Reader(reader);
                        return this.decode(reader, reader.uint32());
                    };
    
                    /**
                     * Verifies a HearthstoneSerializedGameSnapshotAuxData message.
                     * @function verify
                     * @memberof squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshotAuxData
                     * @static
                     * @param {Object.<string,*>} message Plain object to verify
                     * @returns {string|null} `null` if valid, otherwise the reason why it is not
                     */
                    HearthstoneSerializedGameSnapshotAuxData.verify = function verify(message) {
                        if (typeof message !== "object" || message === null)
                            return "object expected";
                        if (message.currentTurn != null && message.hasOwnProperty("currentTurn"))
                            if (!$util.isInteger(message.currentTurn))
                                return "currentTurn: integer expected";
                        if (message.step != null && message.hasOwnProperty("step"))
                            if (!$util.isInteger(message.step))
                                return "step: integer expected";
                        if (message.currentPlayerId != null && message.hasOwnProperty("currentPlayerId"))
                            if (!$util.isInteger(message.currentPlayerId))
                                return "currentPlayerId: integer expected";
                        if (message.lastActionIndex != null && message.hasOwnProperty("lastActionIndex"))
                            if (!$util.isInteger(message.lastActionIndex) && !(message.lastActionIndex && $util.isInteger(message.lastActionIndex.low) && $util.isInteger(message.lastActionIndex.high)))
                                return "lastActionIndex: integer|Long expected";
                        return null;
                    };
    
                    /**
                     * Creates a HearthstoneSerializedGameSnapshotAuxData message from a plain object. Also converts values to their respective internal types.
                     * @function fromObject
                     * @memberof squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshotAuxData
                     * @static
                     * @param {Object.<string,*>} object Plain object
                     * @returns {squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshotAuxData} HearthstoneSerializedGameSnapshotAuxData
                     */
                    HearthstoneSerializedGameSnapshotAuxData.fromObject = function fromObject(object) {
                        if (object instanceof $root.squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshotAuxData)
                            return object;
                        var message = new $root.squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshotAuxData();
                        if (object.currentTurn != null)
                            message.currentTurn = object.currentTurn | 0;
                        if (object.step != null)
                            message.step = object.step | 0;
                        if (object.currentPlayerId != null)
                            message.currentPlayerId = object.currentPlayerId | 0;
                        if (object.lastActionIndex != null)
                            if ($util.Long)
                                (message.lastActionIndex = $util.Long.fromValue(object.lastActionIndex)).unsigned = true;
                            else if (typeof object.lastActionIndex === "string")
                                message.lastActionIndex = parseInt(object.lastActionIndex, 10);
                            else if (typeof object.lastActionIndex === "number")
                                message.lastActionIndex = object.lastActionIndex;
                            else if (typeof object.lastActionIndex === "object")
                                message.lastActionIndex = new $util.LongBits(object.lastActionIndex.low >>> 0, object.lastActionIndex.high >>> 0).toNumber(true);
                        return message;
                    };
    
                    /**
                     * Creates a plain object from a HearthstoneSerializedGameSnapshotAuxData message. Also converts values to other types if specified.
                     * @function toObject
                     * @memberof squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshotAuxData
                     * @static
                     * @param {squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshotAuxData} message HearthstoneSerializedGameSnapshotAuxData
                     * @param {$protobuf.IConversionOptions} [options] Conversion options
                     * @returns {Object.<string,*>} Plain object
                     */
                    HearthstoneSerializedGameSnapshotAuxData.toObject = function toObject(message, options) {
                        if (!options)
                            options = {};
                        var object = {};
                        if (options.defaults) {
                            object.currentTurn = 0;
                            object.step = 0;
                            object.currentPlayerId = 0;
                            if ($util.Long) {
                                var long = new $util.Long(0, 0, true);
                                object.lastActionIndex = options.longs === String ? long.toString() : options.longs === Number ? long.toNumber() : long;
                            } else
                                object.lastActionIndex = options.longs === String ? "0" : 0;
                        }
                        if (message.currentTurn != null && message.hasOwnProperty("currentTurn"))
                            object.currentTurn = message.currentTurn;
                        if (message.step != null && message.hasOwnProperty("step"))
                            object.step = message.step;
                        if (message.currentPlayerId != null && message.hasOwnProperty("currentPlayerId"))
                            object.currentPlayerId = message.currentPlayerId;
                        if (message.lastActionIndex != null && message.hasOwnProperty("lastActionIndex"))
                            if (typeof message.lastActionIndex === "number")
                                object.lastActionIndex = options.longs === String ? String(message.lastActionIndex) : message.lastActionIndex;
                            else
                                object.lastActionIndex = options.longs === String ? $util.Long.prototype.toString.call(message.lastActionIndex) : options.longs === Number ? new $util.LongBits(message.lastActionIndex.low >>> 0, message.lastActionIndex.high >>> 0).toNumber(true) : message.lastActionIndex;
                        return object;
                    };
    
                    /**
                     * Converts this HearthstoneSerializedGameSnapshotAuxData to JSON.
                     * @function toJSON
                     * @memberof squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshotAuxData
                     * @instance
                     * @returns {Object.<string,*>} JSON object
                     */
                    HearthstoneSerializedGameSnapshotAuxData.prototype.toJSON = function toJSON() {
                        return this.constructor.toObject(this, $protobuf.util.toJSONOptions);
                    };
    
                    return HearthstoneSerializedGameSnapshotAuxData;
                })();
    
                game_state.HearthstoneSerializedEntity = (function() {
    
                    /**
                     * Properties of a HearthstoneSerializedEntity.
                     * @memberof squadov.hearthstone.game_state
                     * @interface IHearthstoneSerializedEntity
                     * @property {number|null} [entityId] HearthstoneSerializedEntity entityId
                     * @property {string|null} [tags] HearthstoneSerializedEntity tags
                     * @property {string|null} [attributes] HearthstoneSerializedEntity attributes
                     */
    
                    /**
                     * Constructs a new HearthstoneSerializedEntity.
                     * @memberof squadov.hearthstone.game_state
                     * @classdesc Represents a HearthstoneSerializedEntity.
                     * @implements IHearthstoneSerializedEntity
                     * @constructor
                     * @param {squadov.hearthstone.game_state.IHearthstoneSerializedEntity=} [properties] Properties to set
                     */
                    function HearthstoneSerializedEntity(properties) {
                        if (properties)
                            for (var keys = Object.keys(properties), i = 0; i < keys.length; ++i)
                                if (properties[keys[i]] != null)
                                    this[keys[i]] = properties[keys[i]];
                    }
    
                    /**
                     * HearthstoneSerializedEntity entityId.
                     * @member {number} entityId
                     * @memberof squadov.hearthstone.game_state.HearthstoneSerializedEntity
                     * @instance
                     */
                    HearthstoneSerializedEntity.prototype.entityId = 0;
    
                    /**
                     * HearthstoneSerializedEntity tags.
                     * @member {string} tags
                     * @memberof squadov.hearthstone.game_state.HearthstoneSerializedEntity
                     * @instance
                     */
                    HearthstoneSerializedEntity.prototype.tags = "";
    
                    /**
                     * HearthstoneSerializedEntity attributes.
                     * @member {string} attributes
                     * @memberof squadov.hearthstone.game_state.HearthstoneSerializedEntity
                     * @instance
                     */
                    HearthstoneSerializedEntity.prototype.attributes = "";
    
                    /**
                     * Creates a new HearthstoneSerializedEntity instance using the specified properties.
                     * @function create
                     * @memberof squadov.hearthstone.game_state.HearthstoneSerializedEntity
                     * @static
                     * @param {squadov.hearthstone.game_state.IHearthstoneSerializedEntity=} [properties] Properties to set
                     * @returns {squadov.hearthstone.game_state.HearthstoneSerializedEntity} HearthstoneSerializedEntity instance
                     */
                    HearthstoneSerializedEntity.create = function create(properties) {
                        return new HearthstoneSerializedEntity(properties);
                    };
    
                    /**
                     * Encodes the specified HearthstoneSerializedEntity message. Does not implicitly {@link squadov.hearthstone.game_state.HearthstoneSerializedEntity.verify|verify} messages.
                     * @function encode
                     * @memberof squadov.hearthstone.game_state.HearthstoneSerializedEntity
                     * @static
                     * @param {squadov.hearthstone.game_state.IHearthstoneSerializedEntity} message HearthstoneSerializedEntity message or plain object to encode
                     * @param {$protobuf.Writer} [writer] Writer to encode to
                     * @returns {$protobuf.Writer} Writer
                     */
                    HearthstoneSerializedEntity.encode = function encode(message, writer) {
                        if (!writer)
                            writer = $Writer.create();
                        if (message.entityId != null && Object.hasOwnProperty.call(message, "entityId"))
                            writer.uint32(/* id 1, wireType 0 =*/8).int32(message.entityId);
                        if (message.tags != null && Object.hasOwnProperty.call(message, "tags"))
                            writer.uint32(/* id 2, wireType 2 =*/18).string(message.tags);
                        if (message.attributes != null && Object.hasOwnProperty.call(message, "attributes"))
                            writer.uint32(/* id 3, wireType 2 =*/26).string(message.attributes);
                        return writer;
                    };
    
                    /**
                     * Encodes the specified HearthstoneSerializedEntity message, length delimited. Does not implicitly {@link squadov.hearthstone.game_state.HearthstoneSerializedEntity.verify|verify} messages.
                     * @function encodeDelimited
                     * @memberof squadov.hearthstone.game_state.HearthstoneSerializedEntity
                     * @static
                     * @param {squadov.hearthstone.game_state.IHearthstoneSerializedEntity} message HearthstoneSerializedEntity message or plain object to encode
                     * @param {$protobuf.Writer} [writer] Writer to encode to
                     * @returns {$protobuf.Writer} Writer
                     */
                    HearthstoneSerializedEntity.encodeDelimited = function encodeDelimited(message, writer) {
                        return this.encode(message, writer).ldelim();
                    };
    
                    /**
                     * Decodes a HearthstoneSerializedEntity message from the specified reader or buffer.
                     * @function decode
                     * @memberof squadov.hearthstone.game_state.HearthstoneSerializedEntity
                     * @static
                     * @param {$protobuf.Reader|Uint8Array} reader Reader or buffer to decode from
                     * @param {number} [length] Message length if known beforehand
                     * @returns {squadov.hearthstone.game_state.HearthstoneSerializedEntity} HearthstoneSerializedEntity
                     * @throws {Error} If the payload is not a reader or valid buffer
                     * @throws {$protobuf.util.ProtocolError} If required fields are missing
                     */
                    HearthstoneSerializedEntity.decode = function decode(reader, length) {
                        if (!(reader instanceof $Reader))
                            reader = $Reader.create(reader);
                        var end = length === undefined ? reader.len : reader.pos + length, message = new $root.squadov.hearthstone.game_state.HearthstoneSerializedEntity();
                        while (reader.pos < end) {
                            var tag = reader.uint32();
                            switch (tag >>> 3) {
                            case 1:
                                message.entityId = reader.int32();
                                break;
                            case 2:
                                message.tags = reader.string();
                                break;
                            case 3:
                                message.attributes = reader.string();
                                break;
                            default:
                                reader.skipType(tag & 7);
                                break;
                            }
                        }
                        return message;
                    };
    
                    /**
                     * Decodes a HearthstoneSerializedEntity message from the specified reader or buffer, length delimited.
                     * @function decodeDelimited
                     * @memberof squadov.hearthstone.game_state.HearthstoneSerializedEntity
                     * @static
                     * @param {$protobuf.Reader|Uint8Array} reader Reader or buffer to decode from
                     * @returns {squadov.hearthstone.game_state.HearthstoneSerializedEntity} HearthstoneSerializedEntity
                     * @throws {Error} If the payload is not a reader or valid buffer
                     * @throws {$protobuf.util.ProtocolError} If required fields are missing
                     */
                    HearthstoneSerializedEntity.decodeDelimited = function decodeDelimited(reader) {
                        if (!(reader instanceof $Reader))
                            reader = new $Reader(reader);
                        return this.decode(reader, reader.uint32());
                    };
    
                    /**
                     * Verifies a HearthstoneSerializedEntity message.
                     * @function verify
                     * @memberof squadov.hearthstone.game_state.HearthstoneSerializedEntity
                     * @static
                     * @param {Object.<string,*>} message Plain object to verify
                     * @returns {string|null} `null` if valid, otherwise the reason why it is not
                     */
                    HearthstoneSerializedEntity.verify = function verify(message) {
                        if (typeof message !== "object" || message === null)
                            return "object expected";
                        if (message.entityId != null && message.hasOwnProperty("entityId"))
                            if (!$util.isInteger(message.entityId))
                                return "entityId: integer expected";
                        if (message.tags != null && message.hasOwnProperty("tags"))
                            if (!$util.isString(message.tags))
                                return "tags: string expected";
                        if (message.attributes != null && message.hasOwnProperty("attributes"))
                            if (!$util.isString(message.attributes))
                                return "attributes: string expected";
                        return null;
                    };
    
                    /**
                     * Creates a HearthstoneSerializedEntity message from a plain object. Also converts values to their respective internal types.
                     * @function fromObject
                     * @memberof squadov.hearthstone.game_state.HearthstoneSerializedEntity
                     * @static
                     * @param {Object.<string,*>} object Plain object
                     * @returns {squadov.hearthstone.game_state.HearthstoneSerializedEntity} HearthstoneSerializedEntity
                     */
                    HearthstoneSerializedEntity.fromObject = function fromObject(object) {
                        if (object instanceof $root.squadov.hearthstone.game_state.HearthstoneSerializedEntity)
                            return object;
                        var message = new $root.squadov.hearthstone.game_state.HearthstoneSerializedEntity();
                        if (object.entityId != null)
                            message.entityId = object.entityId | 0;
                        if (object.tags != null)
                            message.tags = String(object.tags);
                        if (object.attributes != null)
                            message.attributes = String(object.attributes);
                        return message;
                    };
    
                    /**
                     * Creates a plain object from a HearthstoneSerializedEntity message. Also converts values to other types if specified.
                     * @function toObject
                     * @memberof squadov.hearthstone.game_state.HearthstoneSerializedEntity
                     * @static
                     * @param {squadov.hearthstone.game_state.HearthstoneSerializedEntity} message HearthstoneSerializedEntity
                     * @param {$protobuf.IConversionOptions} [options] Conversion options
                     * @returns {Object.<string,*>} Plain object
                     */
                    HearthstoneSerializedEntity.toObject = function toObject(message, options) {
                        if (!options)
                            options = {};
                        var object = {};
                        if (options.defaults) {
                            object.entityId = 0;
                            object.tags = "";
                            object.attributes = "";
                        }
                        if (message.entityId != null && message.hasOwnProperty("entityId"))
                            object.entityId = message.entityId;
                        if (message.tags != null && message.hasOwnProperty("tags"))
                            object.tags = message.tags;
                        if (message.attributes != null && message.hasOwnProperty("attributes"))
                            object.attributes = message.attributes;
                        return object;
                    };
    
                    /**
                     * Converts this HearthstoneSerializedEntity to JSON.
                     * @function toJSON
                     * @memberof squadov.hearthstone.game_state.HearthstoneSerializedEntity
                     * @instance
                     * @returns {Object.<string,*>} JSON object
                     */
                    HearthstoneSerializedEntity.prototype.toJSON = function toJSON() {
                        return this.constructor.toObject(this, $protobuf.util.toJSONOptions);
                    };
    
                    return HearthstoneSerializedEntity;
                })();
    
                game_state.HearthstoneSerializedGameSnapshot = (function() {
    
                    /**
                     * Properties of a HearthstoneSerializedGameSnapshot.
                     * @memberof squadov.hearthstone.game_state
                     * @interface IHearthstoneSerializedGameSnapshot
                     * @property {string|null} [uuid] HearthstoneSerializedGameSnapshot uuid
                     * @property {number|Long|null} [tm] HearthstoneSerializedGameSnapshot tm
                     * @property {number|null} [gameEntityId] HearthstoneSerializedGameSnapshot gameEntityId
                     * @property {Object.<string,number>|null} [playerNameToPlayerId] HearthstoneSerializedGameSnapshot playerNameToPlayerId
                     * @property {Object.<string,number>|null} [playerIdToEntityId] HearthstoneSerializedGameSnapshot playerIdToEntityId
                     * @property {Object.<string,squadov.hearthstone.game_state.IHearthstoneSerializedEntity>|null} [entities] HearthstoneSerializedGameSnapshot entities
                     * @property {squadov.hearthstone.game_state.IHearthstoneSerializedGameSnapshotAuxData|null} [auxData] HearthstoneSerializedGameSnapshot auxData
                     */
    
                    /**
                     * Constructs a new HearthstoneSerializedGameSnapshot.
                     * @memberof squadov.hearthstone.game_state
                     * @classdesc Represents a HearthstoneSerializedGameSnapshot.
                     * @implements IHearthstoneSerializedGameSnapshot
                     * @constructor
                     * @param {squadov.hearthstone.game_state.IHearthstoneSerializedGameSnapshot=} [properties] Properties to set
                     */
                    function HearthstoneSerializedGameSnapshot(properties) {
                        this.playerNameToPlayerId = {};
                        this.playerIdToEntityId = {};
                        this.entities = {};
                        if (properties)
                            for (var keys = Object.keys(properties), i = 0; i < keys.length; ++i)
                                if (properties[keys[i]] != null)
                                    this[keys[i]] = properties[keys[i]];
                    }
    
                    /**
                     * HearthstoneSerializedGameSnapshot uuid.
                     * @member {string} uuid
                     * @memberof squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshot
                     * @instance
                     */
                    HearthstoneSerializedGameSnapshot.prototype.uuid = "";
    
                    /**
                     * HearthstoneSerializedGameSnapshot tm.
                     * @member {number|Long} tm
                     * @memberof squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshot
                     * @instance
                     */
                    HearthstoneSerializedGameSnapshot.prototype.tm = $util.Long ? $util.Long.fromBits(0,0,false) : 0;
    
                    /**
                     * HearthstoneSerializedGameSnapshot gameEntityId.
                     * @member {number} gameEntityId
                     * @memberof squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshot
                     * @instance
                     */
                    HearthstoneSerializedGameSnapshot.prototype.gameEntityId = 0;
    
                    /**
                     * HearthstoneSerializedGameSnapshot playerNameToPlayerId.
                     * @member {Object.<string,number>} playerNameToPlayerId
                     * @memberof squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshot
                     * @instance
                     */
                    HearthstoneSerializedGameSnapshot.prototype.playerNameToPlayerId = $util.emptyObject;
    
                    /**
                     * HearthstoneSerializedGameSnapshot playerIdToEntityId.
                     * @member {Object.<string,number>} playerIdToEntityId
                     * @memberof squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshot
                     * @instance
                     */
                    HearthstoneSerializedGameSnapshot.prototype.playerIdToEntityId = $util.emptyObject;
    
                    /**
                     * HearthstoneSerializedGameSnapshot entities.
                     * @member {Object.<string,squadov.hearthstone.game_state.IHearthstoneSerializedEntity>} entities
                     * @memberof squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshot
                     * @instance
                     */
                    HearthstoneSerializedGameSnapshot.prototype.entities = $util.emptyObject;
    
                    /**
                     * HearthstoneSerializedGameSnapshot auxData.
                     * @member {squadov.hearthstone.game_state.IHearthstoneSerializedGameSnapshotAuxData|null|undefined} auxData
                     * @memberof squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshot
                     * @instance
                     */
                    HearthstoneSerializedGameSnapshot.prototype.auxData = null;
    
                    /**
                     * Creates a new HearthstoneSerializedGameSnapshot instance using the specified properties.
                     * @function create
                     * @memberof squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshot
                     * @static
                     * @param {squadov.hearthstone.game_state.IHearthstoneSerializedGameSnapshot=} [properties] Properties to set
                     * @returns {squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshot} HearthstoneSerializedGameSnapshot instance
                     */
                    HearthstoneSerializedGameSnapshot.create = function create(properties) {
                        return new HearthstoneSerializedGameSnapshot(properties);
                    };
    
                    /**
                     * Encodes the specified HearthstoneSerializedGameSnapshot message. Does not implicitly {@link squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshot.verify|verify} messages.
                     * @function encode
                     * @memberof squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshot
                     * @static
                     * @param {squadov.hearthstone.game_state.IHearthstoneSerializedGameSnapshot} message HearthstoneSerializedGameSnapshot message or plain object to encode
                     * @param {$protobuf.Writer} [writer] Writer to encode to
                     * @returns {$protobuf.Writer} Writer
                     */
                    HearthstoneSerializedGameSnapshot.encode = function encode(message, writer) {
                        if (!writer)
                            writer = $Writer.create();
                        if (message.uuid != null && Object.hasOwnProperty.call(message, "uuid"))
                            writer.uint32(/* id 1, wireType 2 =*/10).string(message.uuid);
                        if (message.tm != null && Object.hasOwnProperty.call(message, "tm"))
                            writer.uint32(/* id 2, wireType 0 =*/16).int64(message.tm);
                        if (message.gameEntityId != null && Object.hasOwnProperty.call(message, "gameEntityId"))
                            writer.uint32(/* id 3, wireType 0 =*/24).int32(message.gameEntityId);
                        if (message.playerNameToPlayerId != null && Object.hasOwnProperty.call(message, "playerNameToPlayerId"))
                            for (var keys = Object.keys(message.playerNameToPlayerId), i = 0; i < keys.length; ++i)
                                writer.uint32(/* id 4, wireType 2 =*/34).fork().uint32(/* id 1, wireType 2 =*/10).string(keys[i]).uint32(/* id 2, wireType 0 =*/16).int32(message.playerNameToPlayerId[keys[i]]).ldelim();
                        if (message.playerIdToEntityId != null && Object.hasOwnProperty.call(message, "playerIdToEntityId"))
                            for (var keys = Object.keys(message.playerIdToEntityId), i = 0; i < keys.length; ++i)
                                writer.uint32(/* id 5, wireType 2 =*/42).fork().uint32(/* id 1, wireType 0 =*/8).int32(keys[i]).uint32(/* id 2, wireType 0 =*/16).int32(message.playerIdToEntityId[keys[i]]).ldelim();
                        if (message.entities != null && Object.hasOwnProperty.call(message, "entities"))
                            for (var keys = Object.keys(message.entities), i = 0; i < keys.length; ++i) {
                                writer.uint32(/* id 6, wireType 2 =*/50).fork().uint32(/* id 1, wireType 0 =*/8).int32(keys[i]);
                                $root.squadov.hearthstone.game_state.HearthstoneSerializedEntity.encode(message.entities[keys[i]], writer.uint32(/* id 2, wireType 2 =*/18).fork()).ldelim().ldelim();
                            }
                        if (message.auxData != null && Object.hasOwnProperty.call(message, "auxData"))
                            $root.squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshotAuxData.encode(message.auxData, writer.uint32(/* id 7, wireType 2 =*/58).fork()).ldelim();
                        return writer;
                    };
    
                    /**
                     * Encodes the specified HearthstoneSerializedGameSnapshot message, length delimited. Does not implicitly {@link squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshot.verify|verify} messages.
                     * @function encodeDelimited
                     * @memberof squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshot
                     * @static
                     * @param {squadov.hearthstone.game_state.IHearthstoneSerializedGameSnapshot} message HearthstoneSerializedGameSnapshot message or plain object to encode
                     * @param {$protobuf.Writer} [writer] Writer to encode to
                     * @returns {$protobuf.Writer} Writer
                     */
                    HearthstoneSerializedGameSnapshot.encodeDelimited = function encodeDelimited(message, writer) {
                        return this.encode(message, writer).ldelim();
                    };
    
                    /**
                     * Decodes a HearthstoneSerializedGameSnapshot message from the specified reader or buffer.
                     * @function decode
                     * @memberof squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshot
                     * @static
                     * @param {$protobuf.Reader|Uint8Array} reader Reader or buffer to decode from
                     * @param {number} [length] Message length if known beforehand
                     * @returns {squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshot} HearthstoneSerializedGameSnapshot
                     * @throws {Error} If the payload is not a reader or valid buffer
                     * @throws {$protobuf.util.ProtocolError} If required fields are missing
                     */
                    HearthstoneSerializedGameSnapshot.decode = function decode(reader, length) {
                        if (!(reader instanceof $Reader))
                            reader = $Reader.create(reader);
                        var end = length === undefined ? reader.len : reader.pos + length, message = new $root.squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshot(), key, value;
                        while (reader.pos < end) {
                            var tag = reader.uint32();
                            switch (tag >>> 3) {
                            case 1:
                                message.uuid = reader.string();
                                break;
                            case 2:
                                message.tm = reader.int64();
                                break;
                            case 3:
                                message.gameEntityId = reader.int32();
                                break;
                            case 4:
                                if (message.playerNameToPlayerId === $util.emptyObject)
                                    message.playerNameToPlayerId = {};
                                var end2 = reader.uint32() + reader.pos;
                                key = "";
                                value = 0;
                                while (reader.pos < end2) {
                                    var tag2 = reader.uint32();
                                    switch (tag2 >>> 3) {
                                    case 1:
                                        key = reader.string();
                                        break;
                                    case 2:
                                        value = reader.int32();
                                        break;
                                    default:
                                        reader.skipType(tag2 & 7);
                                        break;
                                    }
                                }
                                message.playerNameToPlayerId[key] = value;
                                break;
                            case 5:
                                if (message.playerIdToEntityId === $util.emptyObject)
                                    message.playerIdToEntityId = {};
                                var end2 = reader.uint32() + reader.pos;
                                key = 0;
                                value = 0;
                                while (reader.pos < end2) {
                                    var tag2 = reader.uint32();
                                    switch (tag2 >>> 3) {
                                    case 1:
                                        key = reader.int32();
                                        break;
                                    case 2:
                                        value = reader.int32();
                                        break;
                                    default:
                                        reader.skipType(tag2 & 7);
                                        break;
                                    }
                                }
                                message.playerIdToEntityId[key] = value;
                                break;
                            case 6:
                                if (message.entities === $util.emptyObject)
                                    message.entities = {};
                                var end2 = reader.uint32() + reader.pos;
                                key = 0;
                                value = null;
                                while (reader.pos < end2) {
                                    var tag2 = reader.uint32();
                                    switch (tag2 >>> 3) {
                                    case 1:
                                        key = reader.int32();
                                        break;
                                    case 2:
                                        value = $root.squadov.hearthstone.game_state.HearthstoneSerializedEntity.decode(reader, reader.uint32());
                                        break;
                                    default:
                                        reader.skipType(tag2 & 7);
                                        break;
                                    }
                                }
                                message.entities[key] = value;
                                break;
                            case 7:
                                message.auxData = $root.squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshotAuxData.decode(reader, reader.uint32());
                                break;
                            default:
                                reader.skipType(tag & 7);
                                break;
                            }
                        }
                        return message;
                    };
    
                    /**
                     * Decodes a HearthstoneSerializedGameSnapshot message from the specified reader or buffer, length delimited.
                     * @function decodeDelimited
                     * @memberof squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshot
                     * @static
                     * @param {$protobuf.Reader|Uint8Array} reader Reader or buffer to decode from
                     * @returns {squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshot} HearthstoneSerializedGameSnapshot
                     * @throws {Error} If the payload is not a reader or valid buffer
                     * @throws {$protobuf.util.ProtocolError} If required fields are missing
                     */
                    HearthstoneSerializedGameSnapshot.decodeDelimited = function decodeDelimited(reader) {
                        if (!(reader instanceof $Reader))
                            reader = new $Reader(reader);
                        return this.decode(reader, reader.uint32());
                    };
    
                    /**
                     * Verifies a HearthstoneSerializedGameSnapshot message.
                     * @function verify
                     * @memberof squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshot
                     * @static
                     * @param {Object.<string,*>} message Plain object to verify
                     * @returns {string|null} `null` if valid, otherwise the reason why it is not
                     */
                    HearthstoneSerializedGameSnapshot.verify = function verify(message) {
                        if (typeof message !== "object" || message === null)
                            return "object expected";
                        if (message.uuid != null && message.hasOwnProperty("uuid"))
                            if (!$util.isString(message.uuid))
                                return "uuid: string expected";
                        if (message.tm != null && message.hasOwnProperty("tm"))
                            if (!$util.isInteger(message.tm) && !(message.tm && $util.isInteger(message.tm.low) && $util.isInteger(message.tm.high)))
                                return "tm: integer|Long expected";
                        if (message.gameEntityId != null && message.hasOwnProperty("gameEntityId"))
                            if (!$util.isInteger(message.gameEntityId))
                                return "gameEntityId: integer expected";
                        if (message.playerNameToPlayerId != null && message.hasOwnProperty("playerNameToPlayerId")) {
                            if (!$util.isObject(message.playerNameToPlayerId))
                                return "playerNameToPlayerId: object expected";
                            var key = Object.keys(message.playerNameToPlayerId);
                            for (var i = 0; i < key.length; ++i)
                                if (!$util.isInteger(message.playerNameToPlayerId[key[i]]))
                                    return "playerNameToPlayerId: integer{k:string} expected";
                        }
                        if (message.playerIdToEntityId != null && message.hasOwnProperty("playerIdToEntityId")) {
                            if (!$util.isObject(message.playerIdToEntityId))
                                return "playerIdToEntityId: object expected";
                            var key = Object.keys(message.playerIdToEntityId);
                            for (var i = 0; i < key.length; ++i) {
                                if (!$util.key32Re.test(key[i]))
                                    return "playerIdToEntityId: integer key{k:int32} expected";
                                if (!$util.isInteger(message.playerIdToEntityId[key[i]]))
                                    return "playerIdToEntityId: integer{k:int32} expected";
                            }
                        }
                        if (message.entities != null && message.hasOwnProperty("entities")) {
                            if (!$util.isObject(message.entities))
                                return "entities: object expected";
                            var key = Object.keys(message.entities);
                            for (var i = 0; i < key.length; ++i) {
                                if (!$util.key32Re.test(key[i]))
                                    return "entities: integer key{k:int32} expected";
                                {
                                    var error = $root.squadov.hearthstone.game_state.HearthstoneSerializedEntity.verify(message.entities[key[i]]);
                                    if (error)
                                        return "entities." + error;
                                }
                            }
                        }
                        if (message.auxData != null && message.hasOwnProperty("auxData")) {
                            var error = $root.squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshotAuxData.verify(message.auxData);
                            if (error)
                                return "auxData." + error;
                        }
                        return null;
                    };
    
                    /**
                     * Creates a HearthstoneSerializedGameSnapshot message from a plain object. Also converts values to their respective internal types.
                     * @function fromObject
                     * @memberof squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshot
                     * @static
                     * @param {Object.<string,*>} object Plain object
                     * @returns {squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshot} HearthstoneSerializedGameSnapshot
                     */
                    HearthstoneSerializedGameSnapshot.fromObject = function fromObject(object) {
                        if (object instanceof $root.squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshot)
                            return object;
                        var message = new $root.squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshot();
                        if (object.uuid != null)
                            message.uuid = String(object.uuid);
                        if (object.tm != null)
                            if ($util.Long)
                                (message.tm = $util.Long.fromValue(object.tm)).unsigned = false;
                            else if (typeof object.tm === "string")
                                message.tm = parseInt(object.tm, 10);
                            else if (typeof object.tm === "number")
                                message.tm = object.tm;
                            else if (typeof object.tm === "object")
                                message.tm = new $util.LongBits(object.tm.low >>> 0, object.tm.high >>> 0).toNumber();
                        if (object.gameEntityId != null)
                            message.gameEntityId = object.gameEntityId | 0;
                        if (object.playerNameToPlayerId) {
                            if (typeof object.playerNameToPlayerId !== "object")
                                throw TypeError(".squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshot.playerNameToPlayerId: object expected");
                            message.playerNameToPlayerId = {};
                            for (var keys = Object.keys(object.playerNameToPlayerId), i = 0; i < keys.length; ++i)
                                message.playerNameToPlayerId[keys[i]] = object.playerNameToPlayerId[keys[i]] | 0;
                        }
                        if (object.playerIdToEntityId) {
                            if (typeof object.playerIdToEntityId !== "object")
                                throw TypeError(".squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshot.playerIdToEntityId: object expected");
                            message.playerIdToEntityId = {};
                            for (var keys = Object.keys(object.playerIdToEntityId), i = 0; i < keys.length; ++i)
                                message.playerIdToEntityId[keys[i]] = object.playerIdToEntityId[keys[i]] | 0;
                        }
                        if (object.entities) {
                            if (typeof object.entities !== "object")
                                throw TypeError(".squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshot.entities: object expected");
                            message.entities = {};
                            for (var keys = Object.keys(object.entities), i = 0; i < keys.length; ++i) {
                                if (typeof object.entities[keys[i]] !== "object")
                                    throw TypeError(".squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshot.entities: object expected");
                                message.entities[keys[i]] = $root.squadov.hearthstone.game_state.HearthstoneSerializedEntity.fromObject(object.entities[keys[i]]);
                            }
                        }
                        if (object.auxData != null) {
                            if (typeof object.auxData !== "object")
                                throw TypeError(".squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshot.auxData: object expected");
                            message.auxData = $root.squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshotAuxData.fromObject(object.auxData);
                        }
                        return message;
                    };
    
                    /**
                     * Creates a plain object from a HearthstoneSerializedGameSnapshot message. Also converts values to other types if specified.
                     * @function toObject
                     * @memberof squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshot
                     * @static
                     * @param {squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshot} message HearthstoneSerializedGameSnapshot
                     * @param {$protobuf.IConversionOptions} [options] Conversion options
                     * @returns {Object.<string,*>} Plain object
                     */
                    HearthstoneSerializedGameSnapshot.toObject = function toObject(message, options) {
                        if (!options)
                            options = {};
                        var object = {};
                        if (options.objects || options.defaults) {
                            object.playerNameToPlayerId = {};
                            object.playerIdToEntityId = {};
                            object.entities = {};
                        }
                        if (options.defaults) {
                            object.uuid = "";
                            if ($util.Long) {
                                var long = new $util.Long(0, 0, false);
                                object.tm = options.longs === String ? long.toString() : options.longs === Number ? long.toNumber() : long;
                            } else
                                object.tm = options.longs === String ? "0" : 0;
                            object.gameEntityId = 0;
                            object.auxData = null;
                        }
                        if (message.uuid != null && message.hasOwnProperty("uuid"))
                            object.uuid = message.uuid;
                        if (message.tm != null && message.hasOwnProperty("tm"))
                            if (typeof message.tm === "number")
                                object.tm = options.longs === String ? String(message.tm) : message.tm;
                            else
                                object.tm = options.longs === String ? $util.Long.prototype.toString.call(message.tm) : options.longs === Number ? new $util.LongBits(message.tm.low >>> 0, message.tm.high >>> 0).toNumber() : message.tm;
                        if (message.gameEntityId != null && message.hasOwnProperty("gameEntityId"))
                            object.gameEntityId = message.gameEntityId;
                        var keys2;
                        if (message.playerNameToPlayerId && (keys2 = Object.keys(message.playerNameToPlayerId)).length) {
                            object.playerNameToPlayerId = {};
                            for (var j = 0; j < keys2.length; ++j)
                                object.playerNameToPlayerId[keys2[j]] = message.playerNameToPlayerId[keys2[j]];
                        }
                        if (message.playerIdToEntityId && (keys2 = Object.keys(message.playerIdToEntityId)).length) {
                            object.playerIdToEntityId = {};
                            for (var j = 0; j < keys2.length; ++j)
                                object.playerIdToEntityId[keys2[j]] = message.playerIdToEntityId[keys2[j]];
                        }
                        if (message.entities && (keys2 = Object.keys(message.entities)).length) {
                            object.entities = {};
                            for (var j = 0; j < keys2.length; ++j)
                                object.entities[keys2[j]] = $root.squadov.hearthstone.game_state.HearthstoneSerializedEntity.toObject(message.entities[keys2[j]], options);
                        }
                        if (message.auxData != null && message.hasOwnProperty("auxData"))
                            object.auxData = $root.squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshotAuxData.toObject(message.auxData, options);
                        return object;
                    };
    
                    /**
                     * Converts this HearthstoneSerializedGameSnapshot to JSON.
                     * @function toJSON
                     * @memberof squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshot
                     * @instance
                     * @returns {Object.<string,*>} JSON object
                     */
                    HearthstoneSerializedGameSnapshot.prototype.toJSON = function toJSON() {
                        return this.constructor.toObject(this, $protobuf.util.toJSONOptions);
                    };
    
                    return HearthstoneSerializedGameSnapshot;
                })();
    
                game_state.HearthstoneSerializedGameBlock = (function() {
    
                    /**
                     * Properties of a HearthstoneSerializedGameBlock.
                     * @memberof squadov.hearthstone.game_state
                     * @interface IHearthstoneSerializedGameBlock
                     * @property {string|null} [blockId] HearthstoneSerializedGameBlock blockId
                     * @property {number|null} [startActionIndex] HearthstoneSerializedGameBlock startActionIndex
                     * @property {number|null} [endActionIndex] HearthstoneSerializedGameBlock endActionIndex
                     * @property {number|null} [blockType] HearthstoneSerializedGameBlock blockType
                     * @property {string|null} [parentBlock] HearthstoneSerializedGameBlock parentBlock
                     * @property {number|null} [entityId] HearthstoneSerializedGameBlock entityId
                     */
    
                    /**
                     * Constructs a new HearthstoneSerializedGameBlock.
                     * @memberof squadov.hearthstone.game_state
                     * @classdesc Represents a HearthstoneSerializedGameBlock.
                     * @implements IHearthstoneSerializedGameBlock
                     * @constructor
                     * @param {squadov.hearthstone.game_state.IHearthstoneSerializedGameBlock=} [properties] Properties to set
                     */
                    function HearthstoneSerializedGameBlock(properties) {
                        if (properties)
                            for (var keys = Object.keys(properties), i = 0; i < keys.length; ++i)
                                if (properties[keys[i]] != null)
                                    this[keys[i]] = properties[keys[i]];
                    }
    
                    /**
                     * HearthstoneSerializedGameBlock blockId.
                     * @member {string} blockId
                     * @memberof squadov.hearthstone.game_state.HearthstoneSerializedGameBlock
                     * @instance
                     */
                    HearthstoneSerializedGameBlock.prototype.blockId = "";
    
                    /**
                     * HearthstoneSerializedGameBlock startActionIndex.
                     * @member {number} startActionIndex
                     * @memberof squadov.hearthstone.game_state.HearthstoneSerializedGameBlock
                     * @instance
                     */
                    HearthstoneSerializedGameBlock.prototype.startActionIndex = 0;
    
                    /**
                     * HearthstoneSerializedGameBlock endActionIndex.
                     * @member {number} endActionIndex
                     * @memberof squadov.hearthstone.game_state.HearthstoneSerializedGameBlock
                     * @instance
                     */
                    HearthstoneSerializedGameBlock.prototype.endActionIndex = 0;
    
                    /**
                     * HearthstoneSerializedGameBlock blockType.
                     * @member {number} blockType
                     * @memberof squadov.hearthstone.game_state.HearthstoneSerializedGameBlock
                     * @instance
                     */
                    HearthstoneSerializedGameBlock.prototype.blockType = 0;
    
                    /**
                     * HearthstoneSerializedGameBlock parentBlock.
                     * @member {string} parentBlock
                     * @memberof squadov.hearthstone.game_state.HearthstoneSerializedGameBlock
                     * @instance
                     */
                    HearthstoneSerializedGameBlock.prototype.parentBlock = "";
    
                    /**
                     * HearthstoneSerializedGameBlock entityId.
                     * @member {number} entityId
                     * @memberof squadov.hearthstone.game_state.HearthstoneSerializedGameBlock
                     * @instance
                     */
                    HearthstoneSerializedGameBlock.prototype.entityId = 0;
    
                    /**
                     * Creates a new HearthstoneSerializedGameBlock instance using the specified properties.
                     * @function create
                     * @memberof squadov.hearthstone.game_state.HearthstoneSerializedGameBlock
                     * @static
                     * @param {squadov.hearthstone.game_state.IHearthstoneSerializedGameBlock=} [properties] Properties to set
                     * @returns {squadov.hearthstone.game_state.HearthstoneSerializedGameBlock} HearthstoneSerializedGameBlock instance
                     */
                    HearthstoneSerializedGameBlock.create = function create(properties) {
                        return new HearthstoneSerializedGameBlock(properties);
                    };
    
                    /**
                     * Encodes the specified HearthstoneSerializedGameBlock message. Does not implicitly {@link squadov.hearthstone.game_state.HearthstoneSerializedGameBlock.verify|verify} messages.
                     * @function encode
                     * @memberof squadov.hearthstone.game_state.HearthstoneSerializedGameBlock
                     * @static
                     * @param {squadov.hearthstone.game_state.IHearthstoneSerializedGameBlock} message HearthstoneSerializedGameBlock message or plain object to encode
                     * @param {$protobuf.Writer} [writer] Writer to encode to
                     * @returns {$protobuf.Writer} Writer
                     */
                    HearthstoneSerializedGameBlock.encode = function encode(message, writer) {
                        if (!writer)
                            writer = $Writer.create();
                        if (message.blockId != null && Object.hasOwnProperty.call(message, "blockId"))
                            writer.uint32(/* id 1, wireType 2 =*/10).string(message.blockId);
                        if (message.startActionIndex != null && Object.hasOwnProperty.call(message, "startActionIndex"))
                            writer.uint32(/* id 2, wireType 0 =*/16).int32(message.startActionIndex);
                        if (message.endActionIndex != null && Object.hasOwnProperty.call(message, "endActionIndex"))
                            writer.uint32(/* id 3, wireType 0 =*/24).int32(message.endActionIndex);
                        if (message.blockType != null && Object.hasOwnProperty.call(message, "blockType"))
                            writer.uint32(/* id 4, wireType 0 =*/32).int32(message.blockType);
                        if (message.parentBlock != null && Object.hasOwnProperty.call(message, "parentBlock"))
                            writer.uint32(/* id 5, wireType 2 =*/42).string(message.parentBlock);
                        if (message.entityId != null && Object.hasOwnProperty.call(message, "entityId"))
                            writer.uint32(/* id 6, wireType 0 =*/48).int32(message.entityId);
                        return writer;
                    };
    
                    /**
                     * Encodes the specified HearthstoneSerializedGameBlock message, length delimited. Does not implicitly {@link squadov.hearthstone.game_state.HearthstoneSerializedGameBlock.verify|verify} messages.
                     * @function encodeDelimited
                     * @memberof squadov.hearthstone.game_state.HearthstoneSerializedGameBlock
                     * @static
                     * @param {squadov.hearthstone.game_state.IHearthstoneSerializedGameBlock} message HearthstoneSerializedGameBlock message or plain object to encode
                     * @param {$protobuf.Writer} [writer] Writer to encode to
                     * @returns {$protobuf.Writer} Writer
                     */
                    HearthstoneSerializedGameBlock.encodeDelimited = function encodeDelimited(message, writer) {
                        return this.encode(message, writer).ldelim();
                    };
    
                    /**
                     * Decodes a HearthstoneSerializedGameBlock message from the specified reader or buffer.
                     * @function decode
                     * @memberof squadov.hearthstone.game_state.HearthstoneSerializedGameBlock
                     * @static
                     * @param {$protobuf.Reader|Uint8Array} reader Reader or buffer to decode from
                     * @param {number} [length] Message length if known beforehand
                     * @returns {squadov.hearthstone.game_state.HearthstoneSerializedGameBlock} HearthstoneSerializedGameBlock
                     * @throws {Error} If the payload is not a reader or valid buffer
                     * @throws {$protobuf.util.ProtocolError} If required fields are missing
                     */
                    HearthstoneSerializedGameBlock.decode = function decode(reader, length) {
                        if (!(reader instanceof $Reader))
                            reader = $Reader.create(reader);
                        var end = length === undefined ? reader.len : reader.pos + length, message = new $root.squadov.hearthstone.game_state.HearthstoneSerializedGameBlock();
                        while (reader.pos < end) {
                            var tag = reader.uint32();
                            switch (tag >>> 3) {
                            case 1:
                                message.blockId = reader.string();
                                break;
                            case 2:
                                message.startActionIndex = reader.int32();
                                break;
                            case 3:
                                message.endActionIndex = reader.int32();
                                break;
                            case 4:
                                message.blockType = reader.int32();
                                break;
                            case 5:
                                message.parentBlock = reader.string();
                                break;
                            case 6:
                                message.entityId = reader.int32();
                                break;
                            default:
                                reader.skipType(tag & 7);
                                break;
                            }
                        }
                        return message;
                    };
    
                    /**
                     * Decodes a HearthstoneSerializedGameBlock message from the specified reader or buffer, length delimited.
                     * @function decodeDelimited
                     * @memberof squadov.hearthstone.game_state.HearthstoneSerializedGameBlock
                     * @static
                     * @param {$protobuf.Reader|Uint8Array} reader Reader or buffer to decode from
                     * @returns {squadov.hearthstone.game_state.HearthstoneSerializedGameBlock} HearthstoneSerializedGameBlock
                     * @throws {Error} If the payload is not a reader or valid buffer
                     * @throws {$protobuf.util.ProtocolError} If required fields are missing
                     */
                    HearthstoneSerializedGameBlock.decodeDelimited = function decodeDelimited(reader) {
                        if (!(reader instanceof $Reader))
                            reader = new $Reader(reader);
                        return this.decode(reader, reader.uint32());
                    };
    
                    /**
                     * Verifies a HearthstoneSerializedGameBlock message.
                     * @function verify
                     * @memberof squadov.hearthstone.game_state.HearthstoneSerializedGameBlock
                     * @static
                     * @param {Object.<string,*>} message Plain object to verify
                     * @returns {string|null} `null` if valid, otherwise the reason why it is not
                     */
                    HearthstoneSerializedGameBlock.verify = function verify(message) {
                        if (typeof message !== "object" || message === null)
                            return "object expected";
                        if (message.blockId != null && message.hasOwnProperty("blockId"))
                            if (!$util.isString(message.blockId))
                                return "blockId: string expected";
                        if (message.startActionIndex != null && message.hasOwnProperty("startActionIndex"))
                            if (!$util.isInteger(message.startActionIndex))
                                return "startActionIndex: integer expected";
                        if (message.endActionIndex != null && message.hasOwnProperty("endActionIndex"))
                            if (!$util.isInteger(message.endActionIndex))
                                return "endActionIndex: integer expected";
                        if (message.blockType != null && message.hasOwnProperty("blockType"))
                            if (!$util.isInteger(message.blockType))
                                return "blockType: integer expected";
                        if (message.parentBlock != null && message.hasOwnProperty("parentBlock"))
                            if (!$util.isString(message.parentBlock))
                                return "parentBlock: string expected";
                        if (message.entityId != null && message.hasOwnProperty("entityId"))
                            if (!$util.isInteger(message.entityId))
                                return "entityId: integer expected";
                        return null;
                    };
    
                    /**
                     * Creates a HearthstoneSerializedGameBlock message from a plain object. Also converts values to their respective internal types.
                     * @function fromObject
                     * @memberof squadov.hearthstone.game_state.HearthstoneSerializedGameBlock
                     * @static
                     * @param {Object.<string,*>} object Plain object
                     * @returns {squadov.hearthstone.game_state.HearthstoneSerializedGameBlock} HearthstoneSerializedGameBlock
                     */
                    HearthstoneSerializedGameBlock.fromObject = function fromObject(object) {
                        if (object instanceof $root.squadov.hearthstone.game_state.HearthstoneSerializedGameBlock)
                            return object;
                        var message = new $root.squadov.hearthstone.game_state.HearthstoneSerializedGameBlock();
                        if (object.blockId != null)
                            message.blockId = String(object.blockId);
                        if (object.startActionIndex != null)
                            message.startActionIndex = object.startActionIndex | 0;
                        if (object.endActionIndex != null)
                            message.endActionIndex = object.endActionIndex | 0;
                        if (object.blockType != null)
                            message.blockType = object.blockType | 0;
                        if (object.parentBlock != null)
                            message.parentBlock = String(object.parentBlock);
                        if (object.entityId != null)
                            message.entityId = object.entityId | 0;
                        return message;
                    };
    
                    /**
                     * Creates a plain object from a HearthstoneSerializedGameBlock message. Also converts values to other types if specified.
                     * @function toObject
                     * @memberof squadov.hearthstone.game_state.HearthstoneSerializedGameBlock
                     * @static
                     * @param {squadov.hearthstone.game_state.HearthstoneSerializedGameBlock} message HearthstoneSerializedGameBlock
                     * @param {$protobuf.IConversionOptions} [options] Conversion options
                     * @returns {Object.<string,*>} Plain object
                     */
                    HearthstoneSerializedGameBlock.toObject = function toObject(message, options) {
                        if (!options)
                            options = {};
                        var object = {};
                        if (options.defaults) {
                            object.blockId = "";
                            object.startActionIndex = 0;
                            object.endActionIndex = 0;
                            object.blockType = 0;
                            object.parentBlock = "";
                            object.entityId = 0;
                        }
                        if (message.blockId != null && message.hasOwnProperty("blockId"))
                            object.blockId = message.blockId;
                        if (message.startActionIndex != null && message.hasOwnProperty("startActionIndex"))
                            object.startActionIndex = message.startActionIndex;
                        if (message.endActionIndex != null && message.hasOwnProperty("endActionIndex"))
                            object.endActionIndex = message.endActionIndex;
                        if (message.blockType != null && message.hasOwnProperty("blockType"))
                            object.blockType = message.blockType;
                        if (message.parentBlock != null && message.hasOwnProperty("parentBlock"))
                            object.parentBlock = message.parentBlock;
                        if (message.entityId != null && message.hasOwnProperty("entityId"))
                            object.entityId = message.entityId;
                        return object;
                    };
    
                    /**
                     * Converts this HearthstoneSerializedGameBlock to JSON.
                     * @function toJSON
                     * @memberof squadov.hearthstone.game_state.HearthstoneSerializedGameBlock
                     * @instance
                     * @returns {Object.<string,*>} JSON object
                     */
                    HearthstoneSerializedGameBlock.prototype.toJSON = function toJSON() {
                        return this.constructor.toObject(this, $protobuf.util.toJSONOptions);
                    };
    
                    return HearthstoneSerializedGameBlock;
                })();
    
                game_state.HearthstoneSerializedGameLog = (function() {
    
                    /**
                     * Properties of a HearthstoneSerializedGameLog.
                     * @memberof squadov.hearthstone.game_state
                     * @interface IHearthstoneSerializedGameLog
                     * @property {Array.<squadov.hearthstone.game_state.IHearthstoneSerializedGameSnapshot>|null} [snapshots] HearthstoneSerializedGameLog snapshots
                     * @property {string|null} [actions] HearthstoneSerializedGameLog actions
                     * @property {Array.<squadov.hearthstone.game_state.IHearthstoneSerializedGameBlock>|null} [blocks] HearthstoneSerializedGameLog blocks
                     */
    
                    /**
                     * Constructs a new HearthstoneSerializedGameLog.
                     * @memberof squadov.hearthstone.game_state
                     * @classdesc Represents a HearthstoneSerializedGameLog.
                     * @implements IHearthstoneSerializedGameLog
                     * @constructor
                     * @param {squadov.hearthstone.game_state.IHearthstoneSerializedGameLog=} [properties] Properties to set
                     */
                    function HearthstoneSerializedGameLog(properties) {
                        this.snapshots = [];
                        this.blocks = [];
                        if (properties)
                            for (var keys = Object.keys(properties), i = 0; i < keys.length; ++i)
                                if (properties[keys[i]] != null)
                                    this[keys[i]] = properties[keys[i]];
                    }
    
                    /**
                     * HearthstoneSerializedGameLog snapshots.
                     * @member {Array.<squadov.hearthstone.game_state.IHearthstoneSerializedGameSnapshot>} snapshots
                     * @memberof squadov.hearthstone.game_state.HearthstoneSerializedGameLog
                     * @instance
                     */
                    HearthstoneSerializedGameLog.prototype.snapshots = $util.emptyArray;
    
                    /**
                     * HearthstoneSerializedGameLog actions.
                     * @member {string} actions
                     * @memberof squadov.hearthstone.game_state.HearthstoneSerializedGameLog
                     * @instance
                     */
                    HearthstoneSerializedGameLog.prototype.actions = "";
    
                    /**
                     * HearthstoneSerializedGameLog blocks.
                     * @member {Array.<squadov.hearthstone.game_state.IHearthstoneSerializedGameBlock>} blocks
                     * @memberof squadov.hearthstone.game_state.HearthstoneSerializedGameLog
                     * @instance
                     */
                    HearthstoneSerializedGameLog.prototype.blocks = $util.emptyArray;
    
                    /**
                     * Creates a new HearthstoneSerializedGameLog instance using the specified properties.
                     * @function create
                     * @memberof squadov.hearthstone.game_state.HearthstoneSerializedGameLog
                     * @static
                     * @param {squadov.hearthstone.game_state.IHearthstoneSerializedGameLog=} [properties] Properties to set
                     * @returns {squadov.hearthstone.game_state.HearthstoneSerializedGameLog} HearthstoneSerializedGameLog instance
                     */
                    HearthstoneSerializedGameLog.create = function create(properties) {
                        return new HearthstoneSerializedGameLog(properties);
                    };
    
                    /**
                     * Encodes the specified HearthstoneSerializedGameLog message. Does not implicitly {@link squadov.hearthstone.game_state.HearthstoneSerializedGameLog.verify|verify} messages.
                     * @function encode
                     * @memberof squadov.hearthstone.game_state.HearthstoneSerializedGameLog
                     * @static
                     * @param {squadov.hearthstone.game_state.IHearthstoneSerializedGameLog} message HearthstoneSerializedGameLog message or plain object to encode
                     * @param {$protobuf.Writer} [writer] Writer to encode to
                     * @returns {$protobuf.Writer} Writer
                     */
                    HearthstoneSerializedGameLog.encode = function encode(message, writer) {
                        if (!writer)
                            writer = $Writer.create();
                        if (message.snapshots != null && message.snapshots.length)
                            for (var i = 0; i < message.snapshots.length; ++i)
                                $root.squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshot.encode(message.snapshots[i], writer.uint32(/* id 1, wireType 2 =*/10).fork()).ldelim();
                        if (message.actions != null && Object.hasOwnProperty.call(message, "actions"))
                            writer.uint32(/* id 2, wireType 2 =*/18).string(message.actions);
                        if (message.blocks != null && message.blocks.length)
                            for (var i = 0; i < message.blocks.length; ++i)
                                $root.squadov.hearthstone.game_state.HearthstoneSerializedGameBlock.encode(message.blocks[i], writer.uint32(/* id 3, wireType 2 =*/26).fork()).ldelim();
                        return writer;
                    };
    
                    /**
                     * Encodes the specified HearthstoneSerializedGameLog message, length delimited. Does not implicitly {@link squadov.hearthstone.game_state.HearthstoneSerializedGameLog.verify|verify} messages.
                     * @function encodeDelimited
                     * @memberof squadov.hearthstone.game_state.HearthstoneSerializedGameLog
                     * @static
                     * @param {squadov.hearthstone.game_state.IHearthstoneSerializedGameLog} message HearthstoneSerializedGameLog message or plain object to encode
                     * @param {$protobuf.Writer} [writer] Writer to encode to
                     * @returns {$protobuf.Writer} Writer
                     */
                    HearthstoneSerializedGameLog.encodeDelimited = function encodeDelimited(message, writer) {
                        return this.encode(message, writer).ldelim();
                    };
    
                    /**
                     * Decodes a HearthstoneSerializedGameLog message from the specified reader or buffer.
                     * @function decode
                     * @memberof squadov.hearthstone.game_state.HearthstoneSerializedGameLog
                     * @static
                     * @param {$protobuf.Reader|Uint8Array} reader Reader or buffer to decode from
                     * @param {number} [length] Message length if known beforehand
                     * @returns {squadov.hearthstone.game_state.HearthstoneSerializedGameLog} HearthstoneSerializedGameLog
                     * @throws {Error} If the payload is not a reader or valid buffer
                     * @throws {$protobuf.util.ProtocolError} If required fields are missing
                     */
                    HearthstoneSerializedGameLog.decode = function decode(reader, length) {
                        if (!(reader instanceof $Reader))
                            reader = $Reader.create(reader);
                        var end = length === undefined ? reader.len : reader.pos + length, message = new $root.squadov.hearthstone.game_state.HearthstoneSerializedGameLog();
                        while (reader.pos < end) {
                            var tag = reader.uint32();
                            switch (tag >>> 3) {
                            case 1:
                                if (!(message.snapshots && message.snapshots.length))
                                    message.snapshots = [];
                                message.snapshots.push($root.squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshot.decode(reader, reader.uint32()));
                                break;
                            case 2:
                                message.actions = reader.string();
                                break;
                            case 3:
                                if (!(message.blocks && message.blocks.length))
                                    message.blocks = [];
                                message.blocks.push($root.squadov.hearthstone.game_state.HearthstoneSerializedGameBlock.decode(reader, reader.uint32()));
                                break;
                            default:
                                reader.skipType(tag & 7);
                                break;
                            }
                        }
                        return message;
                    };
    
                    /**
                     * Decodes a HearthstoneSerializedGameLog message from the specified reader or buffer, length delimited.
                     * @function decodeDelimited
                     * @memberof squadov.hearthstone.game_state.HearthstoneSerializedGameLog
                     * @static
                     * @param {$protobuf.Reader|Uint8Array} reader Reader or buffer to decode from
                     * @returns {squadov.hearthstone.game_state.HearthstoneSerializedGameLog} HearthstoneSerializedGameLog
                     * @throws {Error} If the payload is not a reader or valid buffer
                     * @throws {$protobuf.util.ProtocolError} If required fields are missing
                     */
                    HearthstoneSerializedGameLog.decodeDelimited = function decodeDelimited(reader) {
                        if (!(reader instanceof $Reader))
                            reader = new $Reader(reader);
                        return this.decode(reader, reader.uint32());
                    };
    
                    /**
                     * Verifies a HearthstoneSerializedGameLog message.
                     * @function verify
                     * @memberof squadov.hearthstone.game_state.HearthstoneSerializedGameLog
                     * @static
                     * @param {Object.<string,*>} message Plain object to verify
                     * @returns {string|null} `null` if valid, otherwise the reason why it is not
                     */
                    HearthstoneSerializedGameLog.verify = function verify(message) {
                        if (typeof message !== "object" || message === null)
                            return "object expected";
                        if (message.snapshots != null && message.hasOwnProperty("snapshots")) {
                            if (!Array.isArray(message.snapshots))
                                return "snapshots: array expected";
                            for (var i = 0; i < message.snapshots.length; ++i) {
                                var error = $root.squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshot.verify(message.snapshots[i]);
                                if (error)
                                    return "snapshots." + error;
                            }
                        }
                        if (message.actions != null && message.hasOwnProperty("actions"))
                            if (!$util.isString(message.actions))
                                return "actions: string expected";
                        if (message.blocks != null && message.hasOwnProperty("blocks")) {
                            if (!Array.isArray(message.blocks))
                                return "blocks: array expected";
                            for (var i = 0; i < message.blocks.length; ++i) {
                                var error = $root.squadov.hearthstone.game_state.HearthstoneSerializedGameBlock.verify(message.blocks[i]);
                                if (error)
                                    return "blocks." + error;
                            }
                        }
                        return null;
                    };
    
                    /**
                     * Creates a HearthstoneSerializedGameLog message from a plain object. Also converts values to their respective internal types.
                     * @function fromObject
                     * @memberof squadov.hearthstone.game_state.HearthstoneSerializedGameLog
                     * @static
                     * @param {Object.<string,*>} object Plain object
                     * @returns {squadov.hearthstone.game_state.HearthstoneSerializedGameLog} HearthstoneSerializedGameLog
                     */
                    HearthstoneSerializedGameLog.fromObject = function fromObject(object) {
                        if (object instanceof $root.squadov.hearthstone.game_state.HearthstoneSerializedGameLog)
                            return object;
                        var message = new $root.squadov.hearthstone.game_state.HearthstoneSerializedGameLog();
                        if (object.snapshots) {
                            if (!Array.isArray(object.snapshots))
                                throw TypeError(".squadov.hearthstone.game_state.HearthstoneSerializedGameLog.snapshots: array expected");
                            message.snapshots = [];
                            for (var i = 0; i < object.snapshots.length; ++i) {
                                if (typeof object.snapshots[i] !== "object")
                                    throw TypeError(".squadov.hearthstone.game_state.HearthstoneSerializedGameLog.snapshots: object expected");
                                message.snapshots[i] = $root.squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshot.fromObject(object.snapshots[i]);
                            }
                        }
                        if (object.actions != null)
                            message.actions = String(object.actions);
                        if (object.blocks) {
                            if (!Array.isArray(object.blocks))
                                throw TypeError(".squadov.hearthstone.game_state.HearthstoneSerializedGameLog.blocks: array expected");
                            message.blocks = [];
                            for (var i = 0; i < object.blocks.length; ++i) {
                                if (typeof object.blocks[i] !== "object")
                                    throw TypeError(".squadov.hearthstone.game_state.HearthstoneSerializedGameLog.blocks: object expected");
                                message.blocks[i] = $root.squadov.hearthstone.game_state.HearthstoneSerializedGameBlock.fromObject(object.blocks[i]);
                            }
                        }
                        return message;
                    };
    
                    /**
                     * Creates a plain object from a HearthstoneSerializedGameLog message. Also converts values to other types if specified.
                     * @function toObject
                     * @memberof squadov.hearthstone.game_state.HearthstoneSerializedGameLog
                     * @static
                     * @param {squadov.hearthstone.game_state.HearthstoneSerializedGameLog} message HearthstoneSerializedGameLog
                     * @param {$protobuf.IConversionOptions} [options] Conversion options
                     * @returns {Object.<string,*>} Plain object
                     */
                    HearthstoneSerializedGameLog.toObject = function toObject(message, options) {
                        if (!options)
                            options = {};
                        var object = {};
                        if (options.arrays || options.defaults) {
                            object.snapshots = [];
                            object.blocks = [];
                        }
                        if (options.defaults)
                            object.actions = "";
                        if (message.snapshots && message.snapshots.length) {
                            object.snapshots = [];
                            for (var j = 0; j < message.snapshots.length; ++j)
                                object.snapshots[j] = $root.squadov.hearthstone.game_state.HearthstoneSerializedGameSnapshot.toObject(message.snapshots[j], options);
                        }
                        if (message.actions != null && message.hasOwnProperty("actions"))
                            object.actions = message.actions;
                        if (message.blocks && message.blocks.length) {
                            object.blocks = [];
                            for (var j = 0; j < message.blocks.length; ++j)
                                object.blocks[j] = $root.squadov.hearthstone.game_state.HearthstoneSerializedGameBlock.toObject(message.blocks[j], options);
                        }
                        return object;
                    };
    
                    /**
                     * Converts this HearthstoneSerializedGameLog to JSON.
                     * @function toJSON
                     * @memberof squadov.hearthstone.game_state.HearthstoneSerializedGameLog
                     * @instance
                     * @returns {Object.<string,*>} JSON object
                     */
                    HearthstoneSerializedGameLog.prototype.toJSON = function toJSON() {
                        return this.constructor.toObject(this, $protobuf.util.toJSONOptions);
                    };
    
                    return HearthstoneSerializedGameLog;
                })();
    
                return game_state;
            })();
    
            return hearthstone;
        })();
    
        return squadov;
    })();

    return $root;
});
