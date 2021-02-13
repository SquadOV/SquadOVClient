class AttributeContainer {
    private _attrs: string[]

    constructor(a: string[]) {
        this._attrs = a
    }

    add(a: string) {
        this._attrs.push(a)
    }

    build(): string {
        return this._attrs.join('\n')
    }
}

interface AliasBaseNode {
    alias: string | null
    node: BaseNode
}

export class BaseNode {
    private _node: string
    private _attributes: AttributeContainer
    private _children: AliasBaseNode[]
    private _params: Map<string, string>

    constructor(node: string) {
        this._node = node
        this._params = new Map()
        this._attributes = new AttributeContainer([])
        this._children = []
    }

    genParams() : string {
        if (this._params.size == 0) {
            return ''
        }

        let params : string[] = []
        for (let [key, value] of this._params) {
            params.push(`${key}: ${value}`)
        }
        return `(${ params.join(', ')})`
    }

    addAliasedChild(c : BaseNode, alias: string | null = null) {
        this._children.push({
            alias: alias,
            node: c
        })
    }

    addParam(key: string, value: string) {
        this._params.set(key, value)
    }

    addAttributes(a : string[]) {
        for (let x of a) {
            this._attributes.add(x)
        }
    }

    build(): string {
        let query: string[] = []
        query.push(`${this._node} ${this.genParams()} {`)
        query.push(this._attributes.build())

        for (let c of this._children) {
            query.push(`${!!c.alias ? `${c.alias}: ` : ''} ${c.node.build()}`)
        }

        query.push('}')
        return query.join('\n')
    }
}

export class BuilderNode<T> extends BaseNode {
    private _parent: T

    constructor(p : T, node: string) {
        super(node)
        this._parent = p
    }

    finish() : T {
        return this._parent
    }
}