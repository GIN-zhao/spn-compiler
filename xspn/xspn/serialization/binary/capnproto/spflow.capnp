@0xf80ce2e0b445849e;

enum SPFlowType {
    real @0;
    interval @1;
    positive @2;
    categorical @3;
    ordinal @4;
    count @5;
    binary @6;
}

enum SPFlowMetaType {
    real @0;
    binary @1;
    discrete @2;
}

struct ProductNode {
    children @0 : List(Int32);
}

struct SumNode {
    children @0: List(Int32);
    weights @1 : List(Float64);
}

struct HistogramLeaf {
    breaks @0 : List(Int32);
    densities @1 : List(Float64);
    binReprPoints @2 : List(Float64);
    type @3: SPFlowType;
    metaType @4: SPFlowMetaType;
    scope @5 : Int32;
}

struct GaussianLeaf {
    mean @0 : Float64;
    stddev @1 : Float64;
    scope @2 : Int32;
}

struct CategoricalLeaf {
    probabilities @0 : List(Float64);
    scope @1 : Int32;
}

struct Node {
    id @0 : Int32;
    rootNode @1 : Bool = false;

    union {
        product @2 : ProductNode;
        sum @3 : SumNode;
        hist @4 : HistogramLeaf;
        gaussian @5 : GaussianLeaf;
        categorical @6 : CategoricalLeaf;
    }
}

struct Model {
    rootNode @0 : Int32;
    name @1 : Text;
    featureType @2 : Text;
    numFeatures @3 : Int32;
    nodes @4 : List(Node);
}

struct JointProbability {
    relativeError @0 : Float64;
    model @1 : Model;
}

struct Query {
    batchSize @0 : Int32;

    # TODO: Make this a union as soon as we support multiple queries here.
    joint @1 : JointProbability;
}

struct Header {
    union {
        model @0 : Model;
        query @1 : Query;
    }
}