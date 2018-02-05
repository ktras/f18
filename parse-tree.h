#ifndef FORTRAN_PARSE_TREE_H_
#define FORTRAN_PARSE_TREE_H_

// Defines the classes used to represent successful reductions of productions
// in the Fortran grammar.  The names and content of these definitions
// adhere closely to the syntax specifications in the language standard (q.v.)
// that are transcribed here and referenced via their requirement numbers.
// The representations of some productions that may also be of use in the
// run-time I/O support library have been isolated into a distinct header file
// (viz. format-specification.h).

#include "format-specification.h"
#include "idioms.h"
#include "indirection.h"
#include "message.h"
#include "position.h"
#include <cinttypes>
#include <list>
#include <optional>
#include <ostream>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

// Parse tree node class types do not have default constructors.  They
// explicitly declare "T() {} = delete;" to make this clear.  This restriction
// avoids what would otherwise become a viral requirement to include
// std::monostate among most std::variant<> discriminated union members.

// Parse tree node class types do not have copy constructors or copy assignment
// operators.  They are explicitly declared "= delete;" to make this clear,
// although a C++ compiler wouldn't default them anyway due to the presence
// of move constructors and move assignments.

// Most non-template classes in this file use these default definitions
// for their move constructor and move assignment operator=, and should
// declare an operator<< for formatting.
#define BOILERPLATE(classname) \
  classname(classname &&) = default; \
  classname &operator=(classname &&) = default; \
  friend std::ostream &operator<<(std::ostream &, const classname &); \
  classname() = delete; \
  classname(const classname &) = delete; \
  classname &operator=(const classname &) = delete

// Empty classes are often used below as alternatives in std::variant<>
// discriminated unions.
#define EMPTY_CLASS(classname) \
  struct classname { \
    classname() {} \
    classname(const classname &) {} \
    classname(classname &&) {} \
    classname &operator=(const classname &) { return *this; }; \
    classname &operator=(classname &&) { return *this; }; \
    friend std::ostream &operator<<(std::ostream &, const classname &); \
  }

// Many classes below simply wrap a std::variant<> discriminated union,
// which is conventionally named "u".
#define UNION_CLASS_BOILERPLATE(classname) \
  BOILERPLATE(classname); \
  template<typename A> classname(A &&x) : u(std::move(x)) {}

// Many other classes below simply wrap a std::tuple<> structure, which
// is conventionally named "t".
#define TUPLE_CLASS_BOILERPLATE(classname) \
  BOILERPLATE(classname); \
  template<typename... Ts> \
  classname(Ts &&... args) : t(std::forward<Ts>(args)...) {}

// Many other classes below simply wrap a single data member, which is
// conventionally named "v".
#define WRAPPER_CLASS_BOILERPLATE(classname, type) \
  BOILERPLATE(classname); \
  classname(type &&x) : v(std::move(x)) {} \
  type v;

#define WRAPPER_CLASS(classname, type) \
  struct classname { \
    WRAPPER_CLASS_BOILERPLATE(classname, type); \
  }

namespace Fortran {

// These are the unavoidable recursively-defined productions of Fortran.
// Some references to the representations of their parses require
// indirection.  The Indirect<> pointer wrapper class is used to
// enforce ownership semantics and non-nullability.
struct SpecificationPart;  // R504
struct ExecutableConstruct;  // R514
struct ActionStmt;  // R515
struct AcImpliedDo;  // R774
struct DataImpliedDo;  // R840
struct Designator;  // R901
struct Variable;  // R902
struct Expr;  // R1001
struct WhereConstruct;  // R1042
struct ForallConstruct;  // R1050
struct InputImpliedDo;  // R1218
struct OutputImpliedDo;  // R1218
struct FormatItems;  // R1303
struct FunctionReference;  // R1520
struct FunctionSubprogram;  // R1529
struct SubroutineSubprogram;  // R1534

// These additional forward references are declared so that the order of
// class definitions in this header file can remain reasonably consistent
// with order of the the requirement productions in the grammar.
struct DerivedTypeDef;  // R726
struct EnumDef;  // R759
struct TypeDeclarationStmt;  // R801
struct AccessStmt;  // R827
struct AllocatableStmt;  // R829
struct AsynchronousStmt;  // R831
struct BindStmt;  // R832
struct CodimensionStmt;  // R834
struct ContiguousStmt;  // R836
struct DataStmt;  // R837
struct DataStmtValue;  // R843
struct DimensionStmt;  // R848
struct IntentStmt;  // R849
struct OptionalStmt;  // R850
struct ParameterStmt;  // R851
struct PointerStmt;  // R853
struct ProtectedStmt;  // R855
struct SaveStmt;  // R856
struct TargetStmt;  // R859
struct ValueStmt;  // R861
struct VolatileStmt;  // R862
struct ImplicitStmt;  // R863
struct ImportStmt;  // R867
struct NamelistStmt;  // R868
struct EquivalenceStmt;  // R870
struct CommonStmt;  // R873
struct Substring;  // R908
struct CharLiteralConstantSubstring;
struct DataReference;  // R911
struct StructureComponent;  // R913
struct CoindexedNamedObject;  // R914
struct TypeParamInquiry;  // R916
struct ArrayElement;  // R917
struct AllocateStmt;  // R927
struct NullifyStmt;  // R939
struct DeallocateStmt;  // R941
struct AssignmentStmt;  // R1032
struct PointerAssignmentStmt;  // R1033
struct WhereStmt;  // R1041, R1045, R1046
struct ForallStmt;  // R1055
struct AssociateConstruct;  // R1102
struct BlockConstruct;  // R1107
struct ChangeTeamConstruct;  // R1111
struct CriticalConstruct;  // R1116
struct DoConstruct;  // R1119
struct LabelDoStmt;  // R1121
struct ConcurrentHeader;  // R1125
struct EndDoStmt;  // R1132
struct CycleStmt;  // R1133
struct IfConstruct;  // R1134
struct IfStmt;  // R1139
struct CaseConstruct;  // R1140
struct SelectRankConstruct;  // R1148
struct SelectTypeConstruct;  // R1152
struct ExitStmt;  // R1156
struct GotoStmt;  // R1157
struct ComputedGotoStmt;  // R1158
struct StopStmt;  // R1160, R1161
struct SyncAllStmt;  // R1164
struct SyncImagesStmt;  // R1166
struct SyncMemoryStmt;  // R1168
struct SyncTeamStmt;  // R1169
struct EventPostStmt;  // R1170, R1171
struct EventWaitStmt;  // R1172, R1173, R1174
struct FormTeamStmt;  // R1175, R1176, R1177
struct LockStmt;  // R1178
struct UnlockStmt;  // R1180
struct OpenStmt;  // R1204
struct CloseStmt;  // R1208
struct ReadStmt;  // R1210
struct WriteStmt;  // R1211
struct PrintStmt;  // R1212
struct WaitStmt;  // R1222
struct BackspaceStmt;  // R1224
struct EndfileStmt;  // R1225
struct RewindStmt;  // R1226
struct FlushStmt;  // R1228
struct InquireStmt;  // R1230
struct FormatStmt;  // R1301
struct MainProgram;  // R1401
struct Module;  // R1404
struct UseStmt;  // R1409
struct Submodule;  // R1416
struct BlockData;  // R1420
struct InterfaceBlock;  // R1501
struct GenericSpec;  // R1508
struct GenericStmt;  // R1510
struct ExternalStmt;  // R1511
struct ProcedureDeclarationStmt;  // R1512
struct IntrinsicStmt;  // R1519
struct Call;  // R1520 & R1521
struct CallStmt;  // R1521
struct ProcedureDesignator;  // R1522
struct ActualArg;  // R1524
struct SeparateModuleSubprogram;  // R1538
struct EntryStmt;  // R1541
struct ReturnStmt;  // R1542
struct StmtFunctionStmt;  // R1544

// Extension and deprecated statements
struct BasedPointerStmt;
struct RedimensionStmt;
struct StructureDef;
struct ArithmeticIfStmt;
struct AssignStmt;
struct AssignedGotoStmt;
struct PauseStmt;

// Implicit definitions of the Standard
using Keyword = std::string;

// R403 scalar-xyz -> xyz
// These template class wrappers correspond to the Standard's modifiers
// scalar-xyz, constant-xzy, int-xzy, default-char-xyz, & logical-xyz.
template<typename A> struct Scalar {
  Scalar(Scalar &&that) = default;
  Scalar(A &&that) : thing(std::move(that)) {}
  Scalar &operator=(Scalar &&) = default;
  A thing;
};

template<typename A> struct Constant {
  Constant(Constant &&that) = default;
  Constant(A &&that) : thing(std::move(that)) {}
  Constant &operator=(Constant &&) = default;
  A thing;
};

template<typename A> struct Integer {
  Integer(Integer &&that) = default;
  Integer(A &&that) : thing(std::move(that)) {}
  Integer &operator=(Integer &&) = default;
  A thing;
};

template<typename A> struct Logical {
  Logical(Logical &&that) = default;
  Logical(A &&that) : thing(std::move(that)) {}
  Logical &operator=(Logical &&) = default;
  A thing;
};

template<typename A> struct DefaultChar {
  DefaultChar(DefaultChar &&that) = default;
  DefaultChar(A &&that) : thing(std::move(that)) {}
  DefaultChar &operator=(DefaultChar &&) = default;
  A thing;
};

using LogicalExpr = Logical<Indirection<Expr>>;  // R1024
using DefaultCharExpr = DefaultChar<Indirection<Expr>>;  // R1025
using IntExpr = Integer<Indirection<Expr>>;  // R1026
using ConstantExpr = Constant<Indirection<Expr>>;  // R1029
using IntConstantExpr = Integer<ConstantExpr>;  // R1031
using ScalarLogicalExpr = Scalar<LogicalExpr>;
using ScalarIntExpr = Scalar<IntExpr>;
using ScalarIntConstantExpr = Scalar<IntConstantExpr>;
using ScalarDefaultCharExpr = Scalar<DefaultCharExpr>;
// R1030 default-char-constant-expr is used in the Standard only as part of
// scalar-default-char-constant-expr.
using ScalarDefaultCharConstantExpr = Scalar<DefaultChar<ConstantExpr>>;

// R611 label -> digit [digit]...
using Label = std::uint64_t;  // validated later, must be in [1..99999]

// A wrapper for xzy-stmt productions that are statements, so that
// source positions and labels have a uniform representation.
template<typename A> struct Statement {
  Statement(Position &&pos, std::optional<long> &&lab, bool &&accept, A &&s)
    : position(std::move(pos)),
      label(std::move(lab)), isLabelInAcceptableField{accept},
      statement(std::move(s)) {}
  Position position;
  std::optional<Label> label;
  bool isLabelInAcceptableField{true};
  A statement;
};

// Error recovery marker
EMPTY_CLASS(ErrorRecovery);

// R513 other-specification-stmt ->
//        access-stmt | allocatable-stmt | asynchronous-stmt | bind-stmt |
//        codimension-stmt | contiguous-stmt | dimension-stmt | external-stmt |
//        intent-stmt | intrinsic-stmt | namelist-stmt | optional-stmt |
//        pointer-stmt | protected-stmt | save-stmt | target-stmt |
//        volatile-stmt | value-stmt | common-stmt | equivalence-stmt
// Extension: (Cray) based POINTER statement
struct OtherSpecificationStmt {
  UNION_CLASS_BOILERPLATE(OtherSpecificationStmt);
  std::variant<Indirection<AccessStmt>, Indirection<AllocatableStmt>,
      Indirection<AsynchronousStmt>, Indirection<BindStmt>,
      Indirection<CodimensionStmt>, Indirection<ContiguousStmt>,
      Indirection<DimensionStmt>, Indirection<ExternalStmt>,
      Indirection<IntentStmt>, Indirection<IntrinsicStmt>,
      Indirection<NamelistStmt>, Indirection<OptionalStmt>,
      Indirection<PointerStmt>, Indirection<ProtectedStmt>,
      Indirection<SaveStmt>, Indirection<TargetStmt>, Indirection<ValueStmt>,
      Indirection<VolatileStmt>, Indirection<CommonStmt>,
      Indirection<EquivalenceStmt>, Indirection<BasedPointerStmt>>
      u;
};

// R508 specification-construct ->
//        derived-type-def | enum-def | generic-stmt | interface-block |
//        parameter-stmt | procedure-declaration-stmt |
//        other-specification-stmt | type-declaration-stmt
struct SpecificationConstruct {
  UNION_CLASS_BOILERPLATE(SpecificationConstruct);
  std::variant<Indirection<DerivedTypeDef>, Indirection<EnumDef>,
      Statement<Indirection<GenericStmt>>, Indirection<InterfaceBlock>,
      Statement<Indirection<ParameterStmt>>,
      Statement<Indirection<ProcedureDeclarationStmt>>,
      Statement<OtherSpecificationStmt>,
      Statement<Indirection<TypeDeclarationStmt>>, Indirection<StructureDef>>
      u;
};

// R506 implicit-part-stmt ->
//         implicit-stmt | parameter-stmt | format-stmt | entry-stmt
struct ImplicitPartStmt {
  UNION_CLASS_BOILERPLATE(ImplicitPartStmt);
  std::variant<Statement<Indirection<ImplicitStmt>>,
      Statement<Indirection<ParameterStmt>>, Statement<Indirection<FormatStmt>>,
      Statement<Indirection<EntryStmt>>>
      u;
};

// R505 implicit-part -> [implicit-part-stmt]... implicit-stmt
WRAPPER_CLASS(ImplicitPart, std::list<ImplicitPartStmt>);

// R507 declaration-construct ->
//        specification-construct | data-stmt | format-stmt |
//        entry-stmt | stmt-function-stmt
struct DeclarationConstruct {
  UNION_CLASS_BOILERPLATE(DeclarationConstruct);
  std::variant<SpecificationConstruct, Statement<Indirection<DataStmt>>,
      Statement<Indirection<FormatStmt>>, Statement<Indirection<EntryStmt>>,
      Statement<Indirection<StmtFunctionStmt>>>
      u;
};

// R504 specification-part -> [use-stmt]... [import-stmt]... [implicit-part]
//                            [declaration-construct]...
// TODO: transfer any statements after the last IMPLICIT (if any)
// from the implicit part to the declaration constructs
struct SpecificationPart {
  TUPLE_CLASS_BOILERPLATE(SpecificationPart);
  std::tuple<std::list<Statement<Indirection<UseStmt>>>,
      std::list<Statement<Indirection<ImportStmt>>>, ImplicitPart,
      std::list<DeclarationConstruct>>
      t;
};

// R512 internal-subprogram -> function-subprogram | subroutine-subprogram
struct InternalSubprogram {
  UNION_CLASS_BOILERPLATE(InternalSubprogram);
  std::variant<Indirection<FunctionSubprogram>,
      Indirection<SubroutineSubprogram>>
      u;
};

// R1543 contains-stmt -> CONTAINS
EMPTY_CLASS(ContainsStmt);

// R511 internal-subprogram-part -> contains-stmt [internal-subprogram]...
struct InternalSubprogramPart {
  TUPLE_CLASS_BOILERPLATE(InternalSubprogramPart);
  std::tuple<Statement<ContainsStmt>, std::list<InternalSubprogram>> t;
};

// R1159 continue-stmt -> CONTINUE
EMPTY_CLASS(ContinueStmt);

// R1163 fail-image-stmt -> FAIL IMAGE
EMPTY_CLASS(FailImageStmt);

// R515 action-stmt ->
//        allocate-stmt | assignment-stmt | backspace-stmt | call-stmt |
//        close-stmt | continue-stmt | cycle-stmt | deallocate-stmt |
//        endfile-stmt | error-stop-stmt | event-post-stmt | event-wait-stmt |
//        exit-stmt | fail-image-stmt | flush-stmt | form-team-stmt |
//        goto-stmt | if-stmt | inquire-stmt | lock-stmt | nullify-stmt |
//        open-stmt | pointer-assignment-stmt | print-stmt | read-stmt |
//        return-stmt | rewind-stmt | stop-stmt | sync-all-stmt |
//        sync-images-stmt | sync-memory-stmt | sync-team-stmt | unlock-stmt |
//        wait-stmt | where-stmt | write-stmt | computed-goto-stmt | forall-stmt
struct ActionStmt {
  UNION_CLASS_BOILERPLATE(ActionStmt);
  std::variant<Indirection<AllocateStmt>, Indirection<AssignmentStmt>,
      Indirection<BackspaceStmt>, Indirection<CallStmt>, Indirection<CloseStmt>,
      ContinueStmt, Indirection<CycleStmt>, Indirection<DeallocateStmt>,
      Indirection<EndfileStmt>, Indirection<EventPostStmt>,
      Indirection<EventWaitStmt>, Indirection<ExitStmt>, FailImageStmt,
      Indirection<FlushStmt>, Indirection<FormTeamStmt>, Indirection<GotoStmt>,
      Indirection<IfStmt>, Indirection<InquireStmt>, Indirection<LockStmt>,
      Indirection<NullifyStmt>, Indirection<OpenStmt>,
      Indirection<PointerAssignmentStmt>, Indirection<PrintStmt>,
      Indirection<ReadStmt>, Indirection<ReturnStmt>, Indirection<RewindStmt>,
      Indirection<StopStmt>, Indirection<SyncAllStmt>,
      Indirection<SyncImagesStmt>, Indirection<SyncMemoryStmt>,
      Indirection<SyncTeamStmt>, Indirection<UnlockStmt>, Indirection<WaitStmt>,
      Indirection<WhereStmt>, Indirection<WriteStmt>,
      Indirection<ComputedGotoStmt>, Indirection<ForallStmt>,
      Indirection<RedimensionStmt>, Indirection<ArithmeticIfStmt>,
      Indirection<AssignStmt>, Indirection<AssignedGotoStmt>,
      Indirection<PauseStmt>>
      u;
};

// R514 executable-construct ->
//        action-stmt | associate-construct | block-construct |
//        case-construct | change-team-construct | critical-construct |
//        do-construct | if-construct | select-rank-construct |
//        select-type-construct | where-construct | forall-construct
struct ExecutableConstruct {
  UNION_CLASS_BOILERPLATE(ExecutableConstruct);
  std::variant<Statement<ActionStmt>, Indirection<AssociateConstruct>,
      Indirection<BlockConstruct>, Indirection<CaseConstruct>,
      Indirection<ChangeTeamConstruct>, Indirection<CriticalConstruct>,
      Statement<Indirection<LabelDoStmt>>, Statement<Indirection<EndDoStmt>>,
      Indirection<DoConstruct>, Indirection<IfConstruct>,
      Indirection<SelectRankConstruct>, Indirection<SelectTypeConstruct>,
      Indirection<WhereConstruct>, Indirection<ForallConstruct>>
      u;
};

// R510 execution-part-construct ->
//        executable-construct | format-stmt | entry-stmt | data-stmt
// Extension (PGI/Intel): also accept NAMELIST in execution part
struct ExecutionPartConstruct {
  UNION_CLASS_BOILERPLATE(ExecutionPartConstruct);
  std::variant<ExecutableConstruct, Statement<Indirection<FormatStmt>>,
      Statement<Indirection<EntryStmt>>, Statement<Indirection<DataStmt>>,
      Statement<Indirection<NamelistStmt>>, ErrorRecovery>
      u;
};

// R509 execution-part -> executable-construct [execution-part-construct]...
using ExecutionPart = std::list<ExecutionPartConstruct>;

// R502 program-unit ->
//        main-program | external-subprogram | module | submodule | block-data
// R503 external-subprogram -> function-subprogram | subroutine-subprogram
struct ProgramUnit {
  UNION_CLASS_BOILERPLATE(ProgramUnit);
  std::variant<Indirection<MainProgram>, Indirection<FunctionSubprogram>,
      Indirection<SubroutineSubprogram>, Indirection<Module>,
      Indirection<Submodule>, Indirection<BlockData>>
      u;
};

// R501 program -> program-unit [program-unit]...
// This is the top-level production.
WRAPPER_CLASS(Program, std::list<ProgramUnit>);

// R603 name -> letter [alphanumeric-character]...
using Name = std::string;

// R606 named-constant -> name
WRAPPER_CLASS(NamedConstant, Name);

// R1003 defined-unary-op -> . letter [letter]... .
// R1023 defined-binary-op -> . letter [letter]... .
// R1414 local-defined-operator -> defined-unary-op | defined-binary-op
// R1415 use-defined-operator -> defined-unary-op | defined-binary-op
// The Name here is stored without the dots; e.g., FOO, not .FOO.
WRAPPER_CLASS(DefinedOpName, Name);

// R609 defined-operator ->
//        defined-unary-op | defined-binary-op | extended-intrinsic-op
// R610 extended-intrinsic-op -> intrinsic-operator
struct DefinedOperator {
  UNION_CLASS_BOILERPLATE(DefinedOperator);
  enum class IntrinsicOperator {  // R608 intrinsic-operator
    Power,
    Multiply,
    Divide,
    Add,
    Subtract,
    Concat,
    LT,
    LE,
    EQ,
    NE,
    GE,
    GT,
    NOT,
    AND,
    OR,
    EQV,
    NEQV
  };
  std::variant<DefinedOpName, IntrinsicOperator> u;
};

// R804 object-name -> name
using ObjectName = Name;

// R867 import-stmt ->
//        IMPORT [[::] import-name-list] |
//        IMPORT , ONLY : import-name-list | IMPORT , NONE | IMPORT , ALL
struct ImportStmt {
  BOILERPLATE(ImportStmt);
  enum class Kind { Default, Only, None, All } kind{Kind::Default};
  ImportStmt(Kind &&k) : kind{k} {}
  ImportStmt(std::list<Name> &&n) : names(std::move(n)) {}
  ImportStmt(Kind &&, std::list<Name> &&);
  std::list<Name> names;
};

// R868 namelist-stmt ->
//        NAMELIST / namelist-group-name / namelist-group-object-list
//        [[,] / namelist-group-name / namelist-group-object-list]...
// R869 namelist-group-object -> variable-name
struct NamelistStmt {
  struct Group {
    TUPLE_CLASS_BOILERPLATE(Group);
    std::tuple<Name, std::list<Name>> t;
  };
  WRAPPER_CLASS_BOILERPLATE(NamelistStmt, std::list<Group>);
};

// R701 type-param-value -> scalar-int-expr | * | :
EMPTY_CLASS(Star);

struct TypeParamValue {
  UNION_CLASS_BOILERPLATE(TypeParamValue);
  EMPTY_CLASS(Deferred);  // :
  std::variant<ScalarIntExpr, Star, Deferred> u;
};

// R706 kind-selector -> ( [KIND =] scalar-int-constant-expr )
struct KindSelector {
  WRAPPER_CLASS_BOILERPLATE(KindSelector, ScalarIntConstantExpr);
  KindSelector(std::uint64_t &&);
};

// R705 integer-type-spec -> INTEGER [kind-selector]
WRAPPER_CLASS(IntegerTypeSpec, std::optional<KindSelector>);

// R723 char-length -> ( type-param-value ) | digit-string
struct CharLength {
  UNION_CLASS_BOILERPLATE(CharLength);
  std::variant<TypeParamValue, int64_t> u;
};

// R722 length-selector -> ( [LEN =] type-param-value ) | * char-length [,]
struct LengthSelector {
  UNION_CLASS_BOILERPLATE(LengthSelector);
  std::variant<TypeParamValue, CharLength> u;
};

// R721 char-selector ->
//        length-selector |
//        ( LEN = type-param-value , KIND = scalar-int-constant-expr ) |
//        ( type-param-value , [KIND =] scalar-int-constant-expr ) |
//        ( KIND = scalar-int-constant-expr [, LEN = type-param-value] )
struct CharSelector {
  UNION_CLASS_BOILERPLATE(CharSelector);
  struct LengthAndKind {
    BOILERPLATE(LengthAndKind);
    LengthAndKind(std::optional<TypeParamValue> &&l, ScalarIntConstantExpr &&k)
      : length(std::move(l)), kind(std::move(k)) {}
    std::optional<TypeParamValue> length;
    ScalarIntConstantExpr kind;
  };
  CharSelector(TypeParamValue &&l, ScalarIntConstantExpr &&k)
    : u{LengthAndKind{std::make_optional(std::move(l)), std::move(k)}} {}
  CharSelector(ScalarIntConstantExpr &&k, std::optional<TypeParamValue> &&l)
    : u{LengthAndKind{std::move(l), std::move(k)}} {}
  std::variant<LengthSelector, LengthAndKind> u;
};

// R704 intrinsic-type-spec ->
//        integer-type-spec | REAL [kind-selector] | DOUBLE PRECISION |
//        COMPLEX [kind-selector] | CHARACTER [char-selector] |
//        LOGICAL [kind-selector]
// Extensions: DOUBLE COMPLEX, NCHARACTER (Kanji)
struct IntrinsicTypeSpec {
  UNION_CLASS_BOILERPLATE(IntrinsicTypeSpec);
  struct Real {
    BOILERPLATE(Real);
    Real(std::optional<KindSelector> &&k) : kind{std::move(k)} {}
    std::optional<KindSelector> kind;
  };
  EMPTY_CLASS(DoublePrecision);
  struct Complex {
    BOILERPLATE(Complex);
    Complex(std::optional<KindSelector> &&k) : kind{std::move(k)} {}
    std::optional<KindSelector> kind;
  };
  struct Character {
    BOILERPLATE(Character);
    Character(std::optional<CharSelector> &&s) : selector{std::move(s)} {}
    std::optional<CharSelector> selector;
  };
  struct Logical {
    BOILERPLATE(Logical);
    Logical(std::optional<KindSelector> &&k) : kind{std::move(k)} {}
    std::optional<KindSelector> kind;
  };
  EMPTY_CLASS(DoubleComplex);
  WRAPPER_CLASS(NCharacter, std::optional<LengthSelector>);
  std::variant<IntegerTypeSpec, Real, DoublePrecision, Complex, Character,
      Logical, DoubleComplex, NCharacter>
      u;
};

// R755 type-param-spec -> [keyword =] type-param-value
struct TypeParamSpec {
  TUPLE_CLASS_BOILERPLATE(TypeParamSpec);
  std::tuple<std::optional<Keyword>, TypeParamValue> t;
};

// R754 derived-type-spec -> type-name [(type-param-spec-list)]
struct DerivedTypeSpec {
  TUPLE_CLASS_BOILERPLATE(DerivedTypeSpec);
  std::tuple<Name, std::list<TypeParamSpec>> t;
};

// R702 type-spec -> intrinsic-type-spec | derived-type-spec
struct TypeSpec {
  UNION_CLASS_BOILERPLATE(TypeSpec);
  std::variant<IntrinsicTypeSpec, DerivedTypeSpec> u;
};

// R703 declaration-type-spec ->
//        intrinsic-type-spec | TYPE ( intrinsic-type-spec ) |
//        TYPE ( derived-type-spec ) | CLASS ( derived-type-spec ) |
//        CLASS ( * ) | TYPE ( * )
// Legacy extension: RECORD /struct/
struct DeclarationTypeSpec {
  UNION_CLASS_BOILERPLATE(DeclarationTypeSpec);
  struct Type {
    BOILERPLATE(Type);
    Type(DerivedTypeSpec &&dt) : derived(std::move(dt)) {}
    DerivedTypeSpec derived;
  };
  struct Class {
    BOILERPLATE(Class);
    Class(DerivedTypeSpec &&dt) : derived(std::move(dt)) {}
    DerivedTypeSpec derived;
  };
  EMPTY_CLASS(ClassStar);
  EMPTY_CLASS(TypeStar);
  WRAPPER_CLASS(Record, Name);
  std::variant<IntrinsicTypeSpec, Type, Class, ClassStar, TypeStar, Record> u;
};

// R709 kind-param -> digit-string | scalar-int-constant-name
struct KindParam {
  UNION_CLASS_BOILERPLATE(KindParam);
  EMPTY_CLASS(Kanji);
  std::variant<std::uint64_t, Scalar<Integer<Constant<Name>>>, Kanji> u;
};

// R707 signed-int-literal-constant -> [sign] int-literal-constant
struct SignedIntLiteralConstant {
  TUPLE_CLASS_BOILERPLATE(SignedIntLiteralConstant);
  std::tuple<std::int64_t, std::optional<KindParam>> t;
};

// R708 int-literal-constant -> digit-string [_ kind-param]
struct IntLiteralConstant {
  TUPLE_CLASS_BOILERPLATE(IntLiteralConstant);
  IntLiteralConstant(std::uint64_t n) : t{n, std::optional<KindParam>{}} {}
  std::tuple<std::uint64_t, std::optional<KindParam>> t;
};

// R712 sign -> + | -
enum class Sign { Positive, Negative };

// R717 exponent -> signed-digit-string
struct ExponentPart {
  TUPLE_CLASS_BOILERPLATE(ExponentPart);
  std::tuple<char, std::int64_t> t;
};

// R714 real-literal-constant ->
//        significand [exponent-letter exponent] [_ kind-param] |
//        digit-string exponent-letter exponent [_ kind-param]
// R715 significand -> digit-string . [digit-string] | . digit-string
struct RealLiteralConstant {
  BOILERPLATE(RealLiteralConstant);
  RealLiteralConstant(std::list<char> &&, std::list<char> &&,
      std::optional<ExponentPart> &&, std::optional<KindParam> &&);
  RealLiteralConstant(std::list<char> &&, std::optional<ExponentPart> &&,
      std::optional<KindParam> &&);
  RealLiteralConstant(
      std::list<char> &&, ExponentPart &&, std::optional<KindParam> &&);
  std::string intPart;
  std::string fraction;
  std::optional<ExponentPart> exponent;
  std::optional<KindParam> kind;
};

// R713 signed-real-literal-constant -> [sign] real-literal-constant
struct SignedRealLiteralConstant {
  TUPLE_CLASS_BOILERPLATE(SignedRealLiteralConstant);
  std::tuple<std::optional<Sign>, RealLiteralConstant> t;
};

// R719 real-part ->
//        signed-int-literal-constant | signed-real-literal-constant |
//        named-constant
// R720 imag-part ->
//        signed-int-literal-constant | signed-real-literal-constant |
//        named-constant
struct ComplexPart {
  UNION_CLASS_BOILERPLATE(ComplexPart);
  std::variant<SignedIntLiteralConstant, SignedRealLiteralConstant,
      NamedConstant>
      u;
};

// R718 complex-literal-constant -> ( real-part , imag-part )
struct ComplexLiteralConstant {
  TUPLE_CLASS_BOILERPLATE(ComplexLiteralConstant);
  std::tuple<ComplexPart, ComplexPart> t;  // real, imaginary
};

// Extension: signed COMPLEX constant
struct SignedComplexLiteralConstant {
  TUPLE_CLASS_BOILERPLATE(SignedComplexLiteralConstant);
  std::tuple<Sign, ComplexLiteralConstant> t;
};

// R724 char-literal-constant ->
//        [kind-param _] ' [rep-char]... ' |
//        [kind-param _] " [rep-char]... "
struct CharLiteralConstant {
  TUPLE_CLASS_BOILERPLATE(CharLiteralConstant);
  std::tuple<std::optional<KindParam>, std::string> t;
  std::string GetString() const { return std::get<std::string>(t); }
};

// extension
struct HollerithLiteralConstant {
  WRAPPER_CLASS_BOILERPLATE(HollerithLiteralConstant, std::string);
  std::string GetString() const { return v; }
};

// R725 logical-literal-constant -> .TRUE. | .FALSE.
WRAPPER_CLASS(LogicalLiteralConstant, bool);

// R764 boz-literal-constant -> binary-constant | octal-constant | hex-constant
// R765 binary-constant -> B ' digit [digit]... ' | B " digit [digit]... "
// R766 octal-constant -> O ' digit [digit]... ' | O " digit [digit]... "
// R767 hex-constant ->
//        Z ' hex-digit [hex-digit]... ' | Z " hex-digit [hex-digit]... "
WRAPPER_CLASS(BOZLiteralConstant, std::uint64_t);

// R605 literal-constant ->
//        int-literal-constant | real-literal-constant |
//        complex-literal-constant | logical-literal-constant |
//        char-literal-constant | boz-literal-constant
struct LiteralConstant {
  UNION_CLASS_BOILERPLATE(LiteralConstant);
  std::variant<HollerithLiteralConstant, IntLiteralConstant,
      RealLiteralConstant, ComplexLiteralConstant, BOZLiteralConstant,
      CharLiteralConstant, LogicalLiteralConstant>
      u;
};

// R604 constant ->  literal-constant | named-constant
// Renamed to dodge a clash with Constant<> template class.
struct ConstantValue {
  UNION_CLASS_BOILERPLATE(ConstantValue);
  std::variant<LiteralConstant, NamedConstant> u;
};

// R807 access-spec -> PUBLIC | PRIVATE
struct AccessSpec {
  enum class Kind { Public, Private };
  WRAPPER_CLASS_BOILERPLATE(AccessSpec, Kind);
};

// R728 type-attr-spec ->
//        ABSTRACT | access-spec | BIND(C) | EXTENDS ( parent-type-name )
EMPTY_CLASS(Abstract);
struct TypeAttrSpec {
  UNION_CLASS_BOILERPLATE(TypeAttrSpec);
  EMPTY_CLASS(BindC);
  struct Extends {
    BOILERPLATE(Extends);
    Extends(Name &&n) : name(std::move(n)) {}
    Name name;
  };
  std::variant<Abstract, AccessSpec, BindC, Extends> u;
};

// R727 derived-type-stmt ->
//        TYPE [[, type-attr-spec-list] ::] type-name [( type-param-name-list )]
struct DerivedTypeStmt {
  TUPLE_CLASS_BOILERPLATE(DerivedTypeStmt);
  std::tuple<std::list<TypeAttrSpec>, Name, std::list<Name>> t;
};

// R731 sequence-stmt -> SEQUENCE
EMPTY_CLASS(SequenceStmt);

// R745 private-components-stmt -> PRIVATE
// R747 binding-private-stmt -> PRIVATE
EMPTY_CLASS(PrivateStmt);

// R729 private-or-sequence -> private-components-stmt | sequence-stmt
struct PrivateOrSequence {
  UNION_CLASS_BOILERPLATE(PrivateOrSequence);
  std::variant<PrivateStmt, SequenceStmt> u;
};

// R733 type-param-decl -> type-param-name [= scalar-int-constant-expr]
struct TypeParamDecl {
  TUPLE_CLASS_BOILERPLATE(TypeParamDecl);
  std::tuple<Name, std::optional<ScalarIntConstantExpr>> t;
};

// R732 type-param-def-stmt ->
//        integer-type-spec , type-param-attr-spec :: type-param-decl-list
// R734 type-param-attr-spec -> KIND | LEN
struct TypeParamDefStmt {
  enum class KindOrLength { Kind, Length };  // R734
  TUPLE_CLASS_BOILERPLATE(TypeParamDefStmt);
  std::tuple<IntegerTypeSpec, KindOrLength, std::list<TypeParamDecl>> t;
};

// R1028 specification-expr -> scalar-int-expr
WRAPPER_CLASS(SpecificationExpr, ScalarIntExpr);

// R816 explicit-shape-spec -> [lower-bound :] upper-bound
// R817 lower-bound -> specification-expr
// R818 upper-bound -> specification-expr
struct ExplicitShapeSpec {
  TUPLE_CLASS_BOILERPLATE(ExplicitShapeSpec);
  std::tuple<std::optional<SpecificationExpr>, SpecificationExpr> t;
};

// R810 deferred-coshape-spec -> :
// deferred-coshape-spec-list is just a count of the colons (i.e., the rank).
WRAPPER_CLASS(DeferredCoshapeSpecList, int);

// R811 explicit-coshape-spec ->
//        [[lower-cobound :] upper-cobound ,]... [lower-cobound :] *
// R812 lower-cobound -> specification-expr
// R813 upper-cobound -> specification-expr
struct ExplicitCoshapeSpec {
  TUPLE_CLASS_BOILERPLATE(ExplicitCoshapeSpec);
  std::tuple<std::list<ExplicitShapeSpec>, std::optional<SpecificationExpr>> t;
};

// R809 coarray-spec -> deferred-coshape-spec-list | explicit-coshape-spec
struct CoarraySpec {
  UNION_CLASS_BOILERPLATE(CoarraySpec);
  std::variant<DeferredCoshapeSpecList, ExplicitCoshapeSpec> u;
};

// R820 deferred-shape-spec -> :
// deferred-shape-spec-list is just a count of the colons (i.e., the rank).
WRAPPER_CLASS(DeferredShapeSpecList, int);

// R740 component-array-spec ->
//        explicit-shape-spec-list | deferred-shape-spec-list
struct ComponentArraySpec {
  UNION_CLASS_BOILERPLATE(ComponentArraySpec);
  std::variant<std::list<ExplicitShapeSpec>, DeferredShapeSpecList> u;
};

// R738 component-attr-spec ->
//        access-spec | ALLOCATABLE |
//        CODIMENSION lbracket coarray-spec rbracket |
//        CONTIGUOUS | DIMENSION ( component-array-spec ) | POINTER
EMPTY_CLASS(Allocatable);
EMPTY_CLASS(Pointer);
EMPTY_CLASS(Contiguous);
struct ComponentAttrSpec {
  UNION_CLASS_BOILERPLATE(ComponentAttrSpec);
  std::variant<AccessSpec, Allocatable, CoarraySpec, Contiguous,
      ComponentArraySpec, Pointer>
      u;
};

// R806 null-init -> function-reference
// TODO check that NULL is still intrinsic
EMPTY_CLASS(NullInit);

// R744 initial-data-target -> designator
using InitialDataTarget = Indirection<Designator>;

// R743 component-initialization ->
//        = constant-expr | => null-init | => initial-data-target
// R805 initialization ->
//        = constant-expr | => null-init | => initial-data-target
// Universal extension: initialization -> / data-stmt-value-list /
struct Initialization {
  UNION_CLASS_BOILERPLATE(Initialization);
  std::variant<ConstantExpr, NullInit, InitialDataTarget,
      std::list<Indirection<DataStmtValue>>>
      u;
};

// R739 component-decl ->
//        component-name [( component-array-spec )]
//        [lbracket coarray-spec rbracket] [* char-length]
//        [component-initialization]
struct ComponentDecl {
  TUPLE_CLASS_BOILERPLATE(ComponentDecl);
  std::tuple<Name, std::optional<ComponentArraySpec>,
      std::optional<CoarraySpec>, std::optional<CharLength>,
      std::optional<Initialization>>
      t;
};

// R737 data-component-def-stmt ->
//        declaration-type-spec [[, component-attr-spec-list] ::]
//        component-decl-list
struct DataComponentDefStmt {
  TUPLE_CLASS_BOILERPLATE(DataComponentDefStmt);
  std::tuple<DeclarationTypeSpec, std::list<ComponentAttrSpec>,
      std::list<ComponentDecl>>
      t;
};

// R742 proc-component-attr-spec ->
//        access-spec | NOPASS | PASS [(arg-name)] | POINTER
EMPTY_CLASS(NoPass);
WRAPPER_CLASS(Pass, std::optional<Name>);
struct ProcComponentAttrSpec {
  UNION_CLASS_BOILERPLATE(ProcComponentAttrSpec);
  std::variant<AccessSpec, NoPass, Pass, Pointer> u;
};

// R1517 proc-pointer-init -> null-init | initial-proc-target
// R1518 initial-proc-target -> procedure-name
struct ProcPointerInit {
  UNION_CLASS_BOILERPLATE(ProcPointerInit);
  std::variant<NullInit, Name> u;
};

// R1513 proc-interface -> interface-name | declaration-type-spec
// R1516 interface-name -> name
struct ProcInterface {
  UNION_CLASS_BOILERPLATE(ProcInterface);
  std::variant<Name, DeclarationTypeSpec> u;
};

// R1515 proc-decl -> procedure-entity-name [=> proc-pointer-init]
struct ProcDecl {
  TUPLE_CLASS_BOILERPLATE(ProcDecl);
  std::tuple<Name, std::optional<ProcPointerInit>> t;
};

// R741 proc-component-def-stmt ->
//        PROCEDURE ( [proc-interface] ) , proc-component-attr-spec-list
//          :: proc-decl-list
struct ProcComponentDefStmt {
  TUPLE_CLASS_BOILERPLATE(ProcComponentDefStmt);
  std::tuple<std::optional<ProcInterface>, std::list<ProcComponentAttrSpec>,
      std::list<ProcDecl>>
      t;
};

// R736 component-def-stmt -> data-component-def-stmt | proc-component-def-stmt
struct ComponentDefStmt {
  UNION_CLASS_BOILERPLATE(ComponentDefStmt);
  std::variant<DataComponentDefStmt, ProcComponentDefStmt
      // , TypeParamDefStmt -- PGI accidental extension, not enabled
      >
      u;
};

// R752 bind-attr ->
//        access-spec | DEFERRED | NON_OVERRIDABLE | NOPASS | PASS [(arg-name)]
struct BindAttr {
  UNION_CLASS_BOILERPLATE(BindAttr);
  EMPTY_CLASS(Deferred);
  EMPTY_CLASS(Non_Overridable);
  std::variant<AccessSpec, Deferred, Non_Overridable, NoPass, Pass> u;
};

// R750 type-bound-proc-decl -> binding-name [=> procedure-name]
struct TypeBoundProcDecl {
  TUPLE_CLASS_BOILERPLATE(TypeBoundProcDecl);
  std::tuple<Name, std::optional<Name>> t;
};

// R749 type-bound-procedure-stmt ->
//        PROCEDURE [[, bind-attr-list] ::] type-bound-proc-decl-list |
//        PROCEDURE ( interface-name ) , bind-attr-list :: binding-name-list
struct TypeBoundProcedureStmt {
  UNION_CLASS_BOILERPLATE(TypeBoundProcedureStmt);
  struct WithoutInterface {
    BOILERPLATE(WithoutInterface);
    WithoutInterface(
        std::list<BindAttr> &&as, std::list<TypeBoundProcDecl> &&ds)
      : attributes(std::move(as)), declarations(std::move(ds)) {}
    std::list<BindAttr> attributes;
    std::list<TypeBoundProcDecl> declarations;
  };
  struct WithInterface {
    BOILERPLATE(WithInterface);
    WithInterface(Name &&n, std::list<BindAttr> &&as, std::list<Name> &&bs)
      : interfaceName(std::move(n)), attributes(std::move(as)),
        bindingNames(std::move(bs)) {}
    Name interfaceName;
    std::list<BindAttr> attributes;
    std::list<Name> bindingNames;
  };
  std::variant<WithoutInterface, WithInterface> u;
};

// R751 type-bound-generic-stmt ->
//        GENERIC [, access-spec] :: generic-spec => binding-name-list
struct TypeBoundGenericStmt {
  TUPLE_CLASS_BOILERPLATE(TypeBoundGenericStmt);
  std::tuple<std::optional<AccessSpec>, Indirection<GenericSpec>,
      std::list<Name>>
      t;
};

// R753 final-procedure-stmt -> FINAL [::] final-subroutine-name-list
WRAPPER_CLASS(FinalProcedureStmt, std::list<Name>);

// R748 type-bound-proc-binding ->
//        type-bound-procedure-stmt | type-bound-generic-stmt |
//        final-procedure-stmt
struct TypeBoundProcBinding {
  UNION_CLASS_BOILERPLATE(TypeBoundProcBinding);
  std::variant<TypeBoundProcedureStmt, TypeBoundGenericStmt, FinalProcedureStmt>
      u;
};

// R746 type-bound-procedure-part ->
//        contains-stmt [binding-private-stmt] [type-bound-proc-binding]...
struct TypeBoundProcedurePart {
  TUPLE_CLASS_BOILERPLATE(TypeBoundProcedurePart);
  std::tuple<Statement<ContainsStmt>, std::optional<Statement<PrivateStmt>>,
      std::list<Statement<TypeBoundProcBinding>>>
      t;
};

// R730 end-type-stmt -> END TYPE [type-name]
WRAPPER_CLASS(EndTypeStmt, std::optional<Name>);

// R726 derived-type-def ->
//        derived-type-stmt [type-param-def-stmt]... [private-or-sequence]...
//        [component-part] [type-bound-procedure-part] end-type-stmt
// R735 component-part -> [component-def-stmt]...
struct DerivedTypeDef {
  TUPLE_CLASS_BOILERPLATE(DerivedTypeDef);
  std::tuple<Statement<DerivedTypeStmt>, std::list<Statement<TypeParamDefStmt>>,
      std::list<Statement<PrivateOrSequence>>,
      std::list<Statement<ComponentDefStmt>>,
      std::optional<TypeBoundProcedurePart>, Statement<EndTypeStmt>>
      t;
};

// R758 component-data-source -> expr | data-target | proc-target
// R1037 data-target -> expr
// R1040 proc-target -> expr | procedure-name | proc-component-ref
WRAPPER_CLASS(ComponentDataSource, Indirection<Expr>);

// R757 component-spec -> [keyword =] component-data-source
struct ComponentSpec {
  TUPLE_CLASS_BOILERPLATE(ComponentSpec);
  std::tuple<std::optional<Keyword>, ComponentDataSource> t;
};

// R756 structure-constructor -> derived-type-spec ( [component-spec-list] )
struct StructureConstructor {
  TUPLE_CLASS_BOILERPLATE(StructureConstructor);
  std::tuple<DerivedTypeSpec, std::list<ComponentSpec>> t;
};

// R760 enum-def-stmt -> ENUM, BIND(C)
EMPTY_CLASS(EnumDefStmt);

// R762 enumerator -> named-constant [= scalar-int-constant-expr]
struct Enumerator {
  TUPLE_CLASS_BOILERPLATE(Enumerator);
  std::tuple<NamedConstant, std::optional<ScalarIntConstantExpr>> t;
};

// R761 enumerator-def-stmt -> ENUMERATOR [::] enumerator-list
WRAPPER_CLASS(EnumeratorDefStmt, std::list<Enumerator>);

// R763 end-enum-stmt -> END ENUM
EMPTY_CLASS(EndEnumStmt);

// R759 enum-def ->
//        enum-def-stmt enumerator-def-stmt [enumerator-def-stmt]...
//        end-enum-stmt
struct EnumDef {
  TUPLE_CLASS_BOILERPLATE(EnumDef);
  std::tuple<Statement<EnumDefStmt>, std::list<Statement<EnumeratorDefStmt>>,
      Statement<EndEnumStmt>>
      t;
};

// R773 ac-value -> expr | ac-implied-do
struct AcValue {
  struct Triplet {  // PGI/Intel extension
    TUPLE_CLASS_BOILERPLATE(Triplet);
    std::tuple<ScalarIntExpr, ScalarIntExpr, std::optional<ScalarIntExpr>> t;
  };
  UNION_CLASS_BOILERPLATE(AcValue);
  std::variant<Triplet, Indirection<Expr>, Indirection<AcImpliedDo>> u;
};

// R770 ac-spec -> type-spec :: | [type-spec ::] ac-value-list
struct AcSpec {
  BOILERPLATE(AcSpec);
  AcSpec(std::optional<Indirection<TypeSpec>> &&ts, std::list<AcValue> &&xs)
    : type(std::move(ts)), values(std::move(xs)) {}
  explicit AcSpec(Indirection<TypeSpec> &&ts) : type{std::move(ts)} {}
  // TODO need Indirection here to compile, don't know why
  std::optional<Indirection<TypeSpec>> type;
  std::list<AcValue> values;
};

// R769 array-constructor -> (/ ac-spec /) | lbracket ac-spec rbracket
WRAPPER_CLASS(ArrayConstructor, AcSpec);

// R1124 do-variable -> scalar-int-variable-name
using DoVariable = Scalar<Integer<Name>>;

template<typename A> struct LoopBounds {
  LoopBounds(LoopBounds &&that) = default;
  LoopBounds(DoVariable &&n, A &&a, A &&z, std::optional<A> &&s)
    : name{std::move(n)}, lower{std::move(a)}, upper{std::move(z)},
      step{std::move(s)} {}
  LoopBounds &operator=(LoopBounds &&) = default;
  DoVariable name;
  A lower, upper;
  std::optional<A> step;
};

// R775 ac-implied-do-control ->
//        [integer-type-spec ::] ac-do-variable = scalar-int-expr ,
//        scalar-int-expr [, scalar-int-expr]
// R776 ac-do-variable -> do-variable
struct AcImpliedDoControl {
  TUPLE_CLASS_BOILERPLATE(AcImpliedDoControl);
  std::tuple<std::optional<IntegerTypeSpec>, LoopBounds<ScalarIntExpr>> t;
};

// R774 ac-implied-do -> ( ac-value-list , ac-implied-do-control )
struct AcImpliedDo {
  TUPLE_CLASS_BOILERPLATE(AcImpliedDo);
  std::tuple<std::list<AcValue>, AcImpliedDoControl> t;
};

// R808 language-binding-spec ->
//        BIND ( C [, NAME = scalar-default-char-constant-expr] )
// R1528 proc-language-binding-spec -> language-binding-spec
WRAPPER_CLASS(
    LanguageBindingSpec, std::optional<ScalarDefaultCharConstantExpr>);

// R852 named-constant-def -> named-constant = constant-expr
struct NamedConstantDef {
  TUPLE_CLASS_BOILERPLATE(NamedConstantDef);
  std::tuple<NamedConstant, ConstantExpr> t;
};

// R851 parameter-stmt -> PARAMETER ( named-constant-def-list )
WRAPPER_CLASS(ParameterStmt, std::list<NamedConstantDef>);

// R819 assumed-shape-spec -> [lower-bound] :
WRAPPER_CLASS(AssumedShapeSpec, std::optional<SpecificationExpr>);

// R821 assumed-implied-spec -> [lower-bound :] *
WRAPPER_CLASS(AssumedImpliedSpec, std::optional<SpecificationExpr>);

// R822 assumed-size-spec -> explicit-shape-spec-list , assumed-implied-spec
struct AssumedSizeSpec {
  TUPLE_CLASS_BOILERPLATE(AssumedSizeSpec);
  std::tuple<std::list<ExplicitShapeSpec>, AssumedImpliedSpec> t;
};

// R823 implied-shape-or-assumed-size-spec -> assumed-implied-spec
// R824 implied-shape-spec -> assumed-implied-spec , assumed-implied-spec-list
// I.e., when the assumed-implied-spec-list has a single item, it constitutes an
// implied-shape-or-assumed-size-spec; otherwise, an implied-shape-spec.
WRAPPER_CLASS(ImpliedShapeSpec, std::list<AssumedImpliedSpec>);

// R825 assumed-rank-spec -> ..
EMPTY_CLASS(AssumedRankSpec);

// R815 array-spec ->
//        explicit-shape-spec-list | assumed-shape-spec-list |
//        deferred-shape-spec-list | assumed-size-spec | implied-shape-spec |
//        implied-shape-or-assumed-size-spec | assumed-rank-spec
struct ArraySpec {
  UNION_CLASS_BOILERPLATE(ArraySpec);
  std::variant<std::list<ExplicitShapeSpec>, std::list<AssumedShapeSpec>,
      DeferredShapeSpecList, AssumedSizeSpec, ImpliedShapeSpec, AssumedRankSpec>
      u;
};

// R826 intent-spec -> IN | OUT | INOUT
struct IntentSpec {
  enum class Intent { In, Out, InOut };
  WRAPPER_CLASS_BOILERPLATE(IntentSpec, Intent);
};

// R802 attr-spec ->
//        access-spec | ALLOCATABLE | ASYNCHRONOUS |
//        CODIMENSION lbracket coarray-spec rbracket | CONTIGUOUS |
//        DIMENSION ( array-spec ) | EXTERNAL | INTENT ( intent-spec ) |
//        INTRINSIC | language-binding-spec | OPTIONAL | PARAMETER | POINTER |
//        PROTECTED | SAVE | TARGET | VALUE | VOLATILE
EMPTY_CLASS(Asynchronous);
EMPTY_CLASS(External);
EMPTY_CLASS(Intrinsic);
EMPTY_CLASS(Optional);
EMPTY_CLASS(Parameter);
EMPTY_CLASS(Protected);
EMPTY_CLASS(Save);
EMPTY_CLASS(Target);
EMPTY_CLASS(Value);
EMPTY_CLASS(Volatile);
struct AttrSpec {
  UNION_CLASS_BOILERPLATE(AttrSpec);
  std::variant<AccessSpec, Allocatable, Asynchronous, CoarraySpec, Contiguous,
      ArraySpec, External, IntentSpec, Intrinsic, LanguageBindingSpec, Optional,
      Parameter, Pointer, Protected, Save, Target, Value, Volatile>
      u;
};

// R803 entity-decl ->
//        object-name [( array-spec )] [lbracket coarray-spec rbracket]
//          [* char-length] [initialization] |
//        function-name [* char-length]
struct EntityDecl {
  TUPLE_CLASS_BOILERPLATE(EntityDecl);
  std::tuple<ObjectName, std::optional<ArraySpec>, std::optional<CoarraySpec>,
      std::optional<CharLength>, std::optional<Initialization>>
      t;
};

// R801 type-declaration-stmt ->
//        declaration-type-spec [[, attr-spec]... ::] entity-decl-list
struct TypeDeclarationStmt {
  TUPLE_CLASS_BOILERPLATE(TypeDeclarationStmt);
  std::tuple<DeclarationTypeSpec, std::list<AttrSpec>, std::list<EntityDecl>> t;
};

// R828 access-id -> access-name | generic-spec
struct AccessId {
  UNION_CLASS_BOILERPLATE(AccessId);
  std::variant<Name, Indirection<GenericSpec>> u;
};

// R827 access-stmt -> access-spec [[::] access-id-list]
struct AccessStmt {
  TUPLE_CLASS_BOILERPLATE(AccessStmt);
  std::tuple<AccessSpec, std::list<AccessId>> t;
};

// R830 allocatable-decl ->
//        object-name [( array-spec )] [lbracket coarray-spec rbracket]
// R860 target-decl ->
//        object-name [( array-spec )] [lbracket coarray-spec rbracket]
struct ObjectDecl {
  TUPLE_CLASS_BOILERPLATE(ObjectDecl);
  std::tuple<ObjectName, std::optional<ArraySpec>, std::optional<CoarraySpec>>
      t;
};

// R829 allocatable-stmt -> ALLOCATABLE [::] allocatable-decl-list
WRAPPER_CLASS(AllocatableStmt, std::list<ObjectDecl>);

// R831 asynchronous-stmt -> ASYNCHRONOUS [::] object-name-list
WRAPPER_CLASS(AsynchronousStmt, std::list<ObjectName>);

// R833 bind-entity -> entity-name | / common-block-name /
struct BindEntity {
  TUPLE_CLASS_BOILERPLATE(BindEntity);
  std::tuple<Name, bool /*COMMON*/> t;
};

// R832 bind-stmt -> language-binding-spec [::] bind-entity-list
struct BindStmt {
  TUPLE_CLASS_BOILERPLATE(BindStmt);
  std::tuple<LanguageBindingSpec, std::list<BindEntity>> t;
};

// R835 codimension-decl -> coarray-name lbracket coarray-spec rbracket
struct CodimensionDecl {
  TUPLE_CLASS_BOILERPLATE(CodimensionDecl);
  std::tuple<Name, CoarraySpec> t;
};

// R834 codimension-stmt -> CODIMENSION [::] codimension-decl-list
WRAPPER_CLASS(CodimensionStmt, std::list<CodimensionDecl>);

// R836 contiguous-stmt -> CONTIGUOUS [::] object-name-list
WRAPPER_CLASS(ContiguousStmt, std::list<ObjectName>);

// R847 constant-subobject -> designator
// R846 int-constant-subobject -> constant-subobject
using ConstantSubobject = Constant<Indirection<Designator>>;

// R845 data-stmt-constant ->
//        scalar-constant | scalar-constant-subobject |
//        signed-int-literal-constant | signed-real-literal-constant |
//        null-init | initial-data-target | structure-constructor
struct DataStmtConstant {
  UNION_CLASS_BOILERPLATE(DataStmtConstant);
  std::variant<Scalar<ConstantValue>, Scalar<ConstantSubobject>,
      SignedIntLiteralConstant, SignedRealLiteralConstant,
      SignedComplexLiteralConstant, NullInit, InitialDataTarget,
      StructureConstructor>
      u;
};

// R844 data-stmt-repeat -> scalar-int-constant | scalar-int-constant-subobject
// R607 int-constant -> constant
// R604 constant -> literal-constant | named-constant
// (only literal-constant -> int-literal-constant applies)
struct DataStmtRepeat {
  UNION_CLASS_BOILERPLATE(DataStmtRepeat);
  std::variant<IntLiteralConstant, Scalar<Integer<NamedConstant>>,
      Scalar<Integer<ConstantSubobject>>>
      u;
};

// R843 data-stmt-value -> [data-stmt-repeat *] data-stmt-constant
struct DataStmtValue {
  TUPLE_CLASS_BOILERPLATE(DataStmtValue);
  std::tuple<std::optional<DataStmtRepeat>, DataStmtConstant> t;
};

// R841 data-i-do-object ->
//        array-element | scalar-structure-component | data-implied-do
struct DataIDoObject {
  UNION_CLASS_BOILERPLATE(DataIDoObject);
  std::variant<Scalar<Indirection<Designator>>, Indirection<DataImpliedDo>> u;
};

// R840 data-implied-do ->
//        ( data-i-do-object-list , [integer-type-spec ::] data-i-do-variable
//        = scalar-int-constant-expr , scalar-int-constant-expr
//        [, scalar-int-constant-expr] )
// R842 data-i-do-variable -> do-variable
struct DataImpliedDo {
  TUPLE_CLASS_BOILERPLATE(DataImpliedDo);
  std::tuple<std::list<DataIDoObject>, std::optional<IntegerTypeSpec>,
      LoopBounds<ScalarIntConstantExpr>>
      t;
};

// R839 data-stmt-object -> variable | data-implied-do
struct DataStmtObject {
  UNION_CLASS_BOILERPLATE(DataStmtObject);
  std::variant<Indirection<Variable>, DataImpliedDo> u;
};

// R838 data-stmt-set -> data-stmt-object-list / data-stmt-value-list /
struct DataStmtSet {
  TUPLE_CLASS_BOILERPLATE(DataStmtSet);
  std::tuple<std::list<DataStmtObject>, std::list<DataStmtValue>> t;
};

// R837 data-stmt -> DATA data-stmt-set [[,] data-stmt-set]...
WRAPPER_CLASS(DataStmt, std::list<DataStmtSet>);

// R848 dimension-stmt ->
//        DIMENSION [::] array-name ( array-spec )
//        [, array-name ( array-spec )]...
struct DimensionStmt {
  struct Declaration {
    TUPLE_CLASS_BOILERPLATE(Declaration);
    std::tuple<Name, ArraySpec> t;
  };
  WRAPPER_CLASS_BOILERPLATE(DimensionStmt, std::list<Declaration>);
};

// R849 intent-stmt -> INTENT ( intent-spec ) [::] dummy-arg-name-list
struct IntentStmt {
  TUPLE_CLASS_BOILERPLATE(IntentStmt);
  std::tuple<IntentSpec, std::list<Name>> t;
};

// R850 optional-stmt -> OPTIONAL [::] dummy-arg-name-list
WRAPPER_CLASS(OptionalStmt, std::list<Name>);

// R854 pointer-decl ->
//        object-name [( deferred-shape-spec-list )] | proc-entity-name
struct PointerDecl {
  TUPLE_CLASS_BOILERPLATE(PointerDecl);
  std::tuple<Name, std::optional<DeferredShapeSpecList>> t;
};

// R853 pointer-stmt -> POINTER [::] pointer-decl-list
WRAPPER_CLASS(PointerStmt, std::list<PointerDecl>);

// R855 protected-stmt -> PROTECTED [::] entity-name-list
WRAPPER_CLASS(ProtectedStmt, std::list<Name>);

// R857 saved-entity -> object-name | proc-pointer-name | / common-block-name /
// R858 proc-pointer-name -> name
struct SavedEntity {
  TUPLE_CLASS_BOILERPLATE(SavedEntity);
  std::tuple<Name, bool /*COMMON*/> t;
};

// R856 save-stmt -> SAVE [[::] saved-entity-list]
WRAPPER_CLASS(SaveStmt, std::list<SavedEntity>);

// R859 target-stmt -> TARGET [::] target-decl-list
WRAPPER_CLASS(TargetStmt, std::list<ObjectDecl>);

// R861 value-stmt -> VALUE [::] dummy-arg-name-list
WRAPPER_CLASS(ValueStmt, std::list<Name>);

// R862 volatile-stmt -> VOLATILE [::] object-name-list
WRAPPER_CLASS(VolatileStmt, std::list<ObjectName>);

// R865 letter-spec -> letter [- letter]
struct LetterSpec {
  TUPLE_CLASS_BOILERPLATE(LetterSpec);
  std::tuple<char, std::optional<char>> t;
};

// R864 implicit-spec -> declaration-type-spec ( letter-spec-list )
struct ImplicitSpec {
  TUPLE_CLASS_BOILERPLATE(ImplicitSpec);
  std::tuple<DeclarationTypeSpec, std::list<LetterSpec>> t;
};

// R863 implicit-stmt ->
//        IMPLICIT implicit-spec-list |
//        IMPLICIT NONE [( [implicit-name-spec-list] )]
// R866 implicit-name-spec -> EXTERNAL | TYPE
struct ImplicitStmt {
  UNION_CLASS_BOILERPLATE(ImplicitStmt);
  enum class ImplicitNoneNameSpec { External, Type };  // R866
  friend std::ostream &operator<<(std::ostream &, ImplicitNoneNameSpec);
  std::variant<std::list<ImplicitSpec>, std::list<ImplicitNoneNameSpec>> u;
};

// R874 common-block-object -> variable-name [( array-spec )]
struct CommonBlockObject {
  TUPLE_CLASS_BOILERPLATE(CommonBlockObject);
  std::tuple<Name, std::optional<ArraySpec>> t;
};

// R873 common-stmt ->
//        COMMON [/ [common-block-name] /] common-block-object-list
//        [[,] / [common-block-name] / common-block-object-list]...
struct CommonStmt {
  struct Block {
    TUPLE_CLASS_BOILERPLATE(Block);
    std::tuple<std::optional<Name>, std::list<CommonBlockObject>> t;
  };
  TUPLE_CLASS_BOILERPLATE(CommonStmt);
  std::tuple<std::optional<std::optional<Name>>,  // TODO: flatten
      std::list<CommonBlockObject>, std::list<Block>>
      t;
};

// R872 equivalence-object -> variable-name | array-element | substring
WRAPPER_CLASS(EquivalenceObject, Indirection<Designator>);

// R870 equivalence-stmt -> EQUIVALENCE equivalence-set-list
// R871 equivalence-set -> ( equivalence-object , equivalence-object-list )
WRAPPER_CLASS(EquivalenceStmt, std::list<std::list<EquivalenceObject>>);

// R910 substring-range -> [scalar-int-expr] : [scalar-int-expr]
struct SubstringRange {
  TUPLE_CLASS_BOILERPLATE(SubstringRange);
  std::tuple<std::optional<ScalarIntExpr>, std::optional<ScalarIntExpr>> t;
};

// R919 subscript -> scalar-int-expr
using Subscript = ScalarIntExpr;

// R921 subscript-triplet -> [subscript] : [subscript] [: stride]
struct SubscriptTriplet {
  TUPLE_CLASS_BOILERPLATE(SubscriptTriplet);
  std::tuple<std::optional<Subscript>, std::optional<Subscript>,
      std::optional<Subscript>>
      t;
};

// R923 vector-subscript -> int-expr
using VectorSubscript = IntExpr;

// R920 section-subscript -> subscript | subscript-triplet | vector-subscript
struct SectionSubscript {
  UNION_CLASS_BOILERPLATE(SectionSubscript);
  bool CanConvertToActualArgument() const;
  ActualArg ConvertToActualArgument();
  std::variant<Subscript, SubscriptTriplet, VectorSubscript> u;
};

// R925 cosubscript -> scalar-int-expr
using Cosubscript = ScalarIntExpr;

// R1115 team-variable -> scalar-variable
using TeamVariable = Scalar<Indirection<Variable>>;

// R926 image-selector-spec ->
//        STAT = stat-variable | TEAM = team-variable |
//        TEAM_NUMBER = scalar-int-expr
struct ImageSelectorSpec {
  WRAPPER_CLASS(Stat, Scalar<Integer<Indirection<Variable>>>);
  WRAPPER_CLASS(Team, TeamVariable);
  WRAPPER_CLASS(Team_Number, ScalarIntExpr);
  UNION_CLASS_BOILERPLATE(ImageSelectorSpec);
  std::variant<Stat, Team, Team_Number> u;
};

// R924 image-selector ->
//        lbracket cosubscript-list [, image-selector-spec-list] rbracket
struct ImageSelector {
  TUPLE_CLASS_BOILERPLATE(ImageSelector);
  std::tuple<std::list<Cosubscript>, std::list<ImageSelectorSpec>> t;
};

// R1001 - R1022 expressions
struct Expr {
  UNION_CLASS_BOILERPLATE(Expr);

  WRAPPER_CLASS(IntrinsicUnary, Indirection<Expr>);
  struct Parentheses : public IntrinsicUnary {
    using IntrinsicUnary::IntrinsicUnary;
  };
  struct UnaryPlus : public IntrinsicUnary {
    using IntrinsicUnary::IntrinsicUnary;
  };
  struct Negate : public IntrinsicUnary {
    using IntrinsicUnary::IntrinsicUnary;
  };
  struct NOT : public IntrinsicUnary {
    using IntrinsicUnary::IntrinsicUnary;
  };

  WRAPPER_CLASS(PercentLoc, Indirection<Variable>);  // %LOC(v) extension

  struct DefinedUnary {
    TUPLE_CLASS_BOILERPLATE(DefinedUnary);
    std::tuple<DefinedOpName, Indirection<Expr>> t;
  };

  struct IntrinsicBinary {
    TUPLE_CLASS_BOILERPLATE(IntrinsicBinary);
    std::tuple<Indirection<Expr>, Indirection<Expr>> t;
  };
  struct Power : public IntrinsicBinary {
    using IntrinsicBinary::IntrinsicBinary;
  };
  struct Multiply : public IntrinsicBinary {
    using IntrinsicBinary::IntrinsicBinary;
  };
  struct Divide : public IntrinsicBinary {
    using IntrinsicBinary::IntrinsicBinary;
  };
  struct Add : public IntrinsicBinary {
    using IntrinsicBinary::IntrinsicBinary;
  };
  struct Subtract : public IntrinsicBinary {
    using IntrinsicBinary::IntrinsicBinary;
  };
  struct Concat : public IntrinsicBinary {
    using IntrinsicBinary::IntrinsicBinary;
  };
  struct LT : public IntrinsicBinary {
    using IntrinsicBinary::IntrinsicBinary;
  };
  struct LE : public IntrinsicBinary {
    using IntrinsicBinary::IntrinsicBinary;
  };
  struct EQ : public IntrinsicBinary {
    using IntrinsicBinary::IntrinsicBinary;
  };
  struct NE : public IntrinsicBinary {
    using IntrinsicBinary::IntrinsicBinary;
  };
  struct GE : public IntrinsicBinary {
    using IntrinsicBinary::IntrinsicBinary;
  };
  struct GT : public IntrinsicBinary {
    using IntrinsicBinary::IntrinsicBinary;
  };
  struct AND : public IntrinsicBinary {
    using IntrinsicBinary::IntrinsicBinary;
  };
  struct OR : public IntrinsicBinary {
    using IntrinsicBinary::IntrinsicBinary;
  };
  struct EQV : public IntrinsicBinary {
    using IntrinsicBinary::IntrinsicBinary;
  };
  struct NEQV : public IntrinsicBinary {
    using IntrinsicBinary::IntrinsicBinary;
  };

  // PGI/XLF extension: (x,y)
  struct ComplexConstructor : public IntrinsicBinary {
    using IntrinsicBinary::IntrinsicBinary;
  };

  struct DefinedBinary {
    TUPLE_CLASS_BOILERPLATE(DefinedBinary);
    std::tuple<DefinedOpName, Indirection<Expr>, Indirection<Expr>> t;
  };

  explicit Expr(Designator &&);
  explicit Expr(FunctionReference &&);

  std::optional<Variable> ConvertToVariable();
  ActualArg ConvertToActualArgument();

  std::variant<Indirection<CharLiteralConstantSubstring>, LiteralConstant,
      Indirection<Designator>, ArrayConstructor, StructureConstructor,
      Indirection<TypeParamInquiry>, Indirection<FunctionReference>,
      Parentheses, UnaryPlus, Negate, NOT, PercentLoc, DefinedUnary, Power,
      Multiply, Divide, Add, Subtract, Concat, LT, LE, EQ, NE, GE, GT, AND, OR,
      EQV, NEQV, DefinedBinary, ComplexConstructor>
      u;
};

// R912 part-ref -> part-name [( section-subscript-list )] [image-selector]
struct PartRef {
  BOILERPLATE(PartRef);
  PartRef(Name &&n, std::list<SectionSubscript> &&ss,
      std::optional<ImageSelector> &&is)
    : name{std::move(n)},
      subscripts(std::move(ss)), imageSelector{std::move(is)} {}
  Name name;
  std::list<SectionSubscript> subscripts;
  std::optional<ImageSelector> imageSelector;
};

// R911 data-ref -> part-ref [% part-ref]...
struct DataReference {
  UNION_CLASS_BOILERPLATE(DataReference);
  explicit DataReference(std::list<PartRef> &&);
  std::variant<Name, Indirection<StructureComponent>, Indirection<ArrayElement>,
      Indirection<CoindexedNamedObject>>
      u;
};

// R908 substring -> parent-string ( substring-range )
// R909 parent-string ->
//        scalar-variable-name | array-element | coindexed-named-object |
//        scalar-structure-component | scalar-char-literal-constant |
//        scalar-named-constant
// Substrings of character literals have been factored out into their
// own productions so that they can't appear as designators in any context
// other than a primary expression.
struct Substring {
  TUPLE_CLASS_BOILERPLATE(Substring);
  std::tuple<DataReference, SubstringRange> t;
};

struct CharLiteralConstantSubstring {
  TUPLE_CLASS_BOILERPLATE(CharLiteralConstantSubstring);
  std::tuple<CharLiteralConstant, SubstringRange> t;
};

// R901 designator -> object-name | array-element | array-section |
//                    coindexed-named-object | complex-part-designator |
//                    structure-component | substring
struct Designator {
  UNION_CLASS_BOILERPLATE(Designator);
  bool EndsInBareName() const;
  ProcedureDesignator ConvertToProcedureDesignator();
  std::optional<Call> ConvertToCall();
  std::variant<ObjectName, DataReference, Substring> u;
};

// R902 variable -> designator | function-reference
struct Variable {
  UNION_CLASS_BOILERPLATE(Variable);
  std::variant<Indirection<Designator>, Indirection<FunctionReference>> u;
};

// R904 logical-variable -> variable
// Appears only as part of scalar-logical-variable.
using ScalarLogicalVariable = Scalar<Logical<Variable>>;

// R905 char-variable -> variable
WRAPPER_CLASS(CharVariable, Variable);

// R906 default-char-variable -> variable
// Appears only as part of scalar-default-char-variable.
using ScalarDefaultCharVariable = Scalar<DefaultChar<Variable>>;

// R907 int-variable -> variable
// Appears only as part of scalar-int-variable.
using ScalarIntVariable = Scalar<Integer<Variable>>;

// R1039 proc-component-ref -> scalar-variable % procedure-component-name
struct ProcComponentRef {
  TUPLE_CLASS_BOILERPLATE(ProcComponentRef);
  std::tuple<Scalar<Variable>, Name> t;
};

// R913 structure-component -> data-ref
struct StructureComponent {
  BOILERPLATE(StructureComponent);
  StructureComponent(DataReference &&dr, Name &&n)
    : base{std::move(dr)}, component(std::move(n)) {}
  DataReference base;
  Name component;
};

// R914 coindexed-named-object -> data-ref
struct CoindexedNamedObject {
  BOILERPLATE(CoindexedNamedObject);
  CoindexedNamedObject(DataReference &&dr, ImageSelector &&is)
    : base{std::move(dr)}, imageSelector{std::move(is)} {}
  DataReference base;
  ImageSelector imageSelector;
};

// R915 complex-part-designator -> designator % RE | designator % IM
struct ComplexPartDesignator {
  WRAPPER_CLASS_BOILERPLATE(ComplexPartDesignator, StructureComponent);
};

// R916 type-param-inquiry -> designator % type-param-name
struct TypeParamInquiry {
  WRAPPER_CLASS_BOILERPLATE(TypeParamInquiry, StructureComponent);
};

// R917 array-element -> data-ref
struct ArrayElement {
  BOILERPLATE(ArrayElement);
  ArrayElement(DataReference &&dr, std::list<SectionSubscript> &&ss)
    : base{std::move(dr)}, subscripts(std::move(ss)) {}
  DataReference base;
  std::list<SectionSubscript> subscripts;
};

// R918 array-section ->
//        data-ref [( substring-range )] | complex-part-designator
struct ArraySection {
  WRAPPER_CLASS_BOILERPLATE(ArraySection, Designator);
  // at least one vector-valued or triplet subscript
};

// R933 allocate-object -> variable-name | structure-component
struct AllocateObject {
  UNION_CLASS_BOILERPLATE(AllocateObject);
  std::variant<Name, StructureComponent> u;
};

// R935 lower-bound-expr -> scalar-int-expr
// R936 upper-bound-expr -> scalar-int-expr
using BoundExpr = ScalarIntExpr;

// R934 allocate-shape-spec -> [lower-bound-expr :] upper-bound-expr
// R938 allocate-coshape-spec -> [lower-bound-expr :] upper-bound-expr
struct AllocateShapeSpec {
  TUPLE_CLASS_BOILERPLATE(AllocateShapeSpec);
  std::tuple<std::optional<BoundExpr>, BoundExpr> t;
};

using AllocateCoshapeSpec = AllocateShapeSpec;

// R937 allocate-coarray-spec ->
//      [allocate-coshape-spec-list ,] [lower-bound-expr :] *
struct AllocateCoarraySpec {
  TUPLE_CLASS_BOILERPLATE(AllocateCoarraySpec);
  std::tuple<std::list<AllocateCoshapeSpec>, std::optional<BoundExpr>> t;
};

// R932 allocation ->
//        allocate-object [( allocate-shape-spec-list )]
//        [lbracket allocate-coarray-spec rbracket]
struct Allocation {
  TUPLE_CLASS_BOILERPLATE(Allocation);
  std::tuple<AllocateObject, std::list<AllocateShapeSpec>,
      std::optional<AllocateCoarraySpec>>
      t;
};

// R929 stat-variable -> scalar-int-variable
WRAPPER_CLASS(StatVariable, ScalarIntVariable);

// R930 errmsg-variable -> scalar-default-char-variable
// R1207 iomsg-variable -> scalar-default-char-variable
WRAPPER_CLASS(MsgVariable, ScalarDefaultCharVariable);

// R942 dealloc-opt -> STAT = stat-variable | ERRMSG = errmsg-variable
// R1165 sync-stat -> STAT = stat-variable | ERRMSG = errmsg-variable
struct StatOrErrmsg {
  UNION_CLASS_BOILERPLATE(StatOrErrmsg);
  std::variant<StatVariable, MsgVariable> u;
};

// R928 alloc-opt ->
//        ERRMSG = errmsg-variable | MOLD = source-expr |
//        SOURCE = source-expr | STAT = stat-variable
// R931 source-expr -> expr
struct AllocOpt {
  UNION_CLASS_BOILERPLATE(AllocOpt);
  WRAPPER_CLASS(Mold, Indirection<Expr>);
  WRAPPER_CLASS(Source, Indirection<Expr>);
  std::variant<Mold, Source, StatOrErrmsg> u;
};

// R927 allocate-stmt ->
//        ALLOCATE ( [type-spec ::] allocation-list [, alloc-opt-list] )
struct AllocateStmt {
  TUPLE_CLASS_BOILERPLATE(AllocateStmt);
  std::tuple<std::optional<TypeSpec>, std::list<Allocation>,
      std::list<AllocOpt>>
      t;
};

// R940 pointer-object ->
//        variable-name | structure-component | proc-pointer-name
struct PointerObject {
  UNION_CLASS_BOILERPLATE(PointerObject);
  std::variant<Name, StructureComponent> u;
};

// R939 nullify-stmt -> NULLIFY ( pointer-object-list )
WRAPPER_CLASS(NullifyStmt, std::list<PointerObject>);

// R941 deallocate-stmt ->
//        DEALLOCATE ( allocate-object-list [, dealloc-opt-list] )
struct DeallocateStmt {
  TUPLE_CLASS_BOILERPLATE(DeallocateStmt);
  std::tuple<std::list<AllocateObject>, std::list<StatOrErrmsg>> t;
};

// R1032 assignment-stmt -> variable = expr
struct AssignmentStmt {
  TUPLE_CLASS_BOILERPLATE(AssignmentStmt);
  std::tuple<Variable, Expr> t;
};

// R1035 bounds-spec -> lower-bound-expr :
WRAPPER_CLASS(BoundsSpec, BoundExpr);

// R1036 bounds-remapping -> lower-bound-expr : upper-bound-expr
struct BoundsRemapping {
  TUPLE_CLASS_BOILERPLATE(BoundsRemapping);
  std::tuple<BoundExpr, BoundExpr> t;
};

// R1033 pointer-assignment-stmt ->
//         data-pointer-object [( bounds-spec-list )] => data-target |
//         data-pointer-object ( bounds-remapping-list ) => data-target |
//         proc-pointer-object => proc-target
// R1034 data-pointer-object ->
//         variable-name | scalar-variable % data-pointer-component-name
// R1038 proc-pointer-object -> proc-pointer-name | proc-component-ref
struct PointerAssignmentStmt {
  struct Bounds {
    UNION_CLASS_BOILERPLATE(Bounds);
    std::variant<std::list<BoundsRemapping>, std::list<BoundsSpec>> u;
  };
  TUPLE_CLASS_BOILERPLATE(PointerAssignmentStmt);
  std::tuple<Variable, Bounds, Expr> t;
};

// R1041 where-stmt -> WHERE ( mask-expr ) where-assignment-stmt
// R1045 where-assignment-stmt -> assignment-stmt
// R1046 mask-expr -> logical-expr
struct WhereStmt {
  TUPLE_CLASS_BOILERPLATE(WhereStmt);
  std::tuple<LogicalExpr, AssignmentStmt> t;
};

// R1043 where-construct-stmt -> [where-construct-name :] WHERE ( mask-expr )
struct WhereConstructStmt {
  TUPLE_CLASS_BOILERPLATE(WhereConstructStmt);
  std::tuple<std::optional<Name>, LogicalExpr> t;
};

// R1044 where-body-construct ->
//         where-assignment-stmt | where-stmt | where-construct
struct WhereBodyConstruct {
  UNION_CLASS_BOILERPLATE(WhereBodyConstruct);
  std::variant<Statement<AssignmentStmt>, Statement<WhereStmt>,
      Indirection<WhereConstruct>>
      u;
};

// R1047 masked-elsewhere-stmt ->
//         ELSEWHERE ( mask-expr ) [where-construct-name]
struct MaskedElsewhereStmt {
  TUPLE_CLASS_BOILERPLATE(MaskedElsewhereStmt);
  std::tuple<LogicalExpr, std::optional<Name>> t;
};

// R1048 elsewhere-stmt -> ELSEWHERE [where-construct-name]
WRAPPER_CLASS(ElsewhereStmt, std::optional<Name>);

// R1049 end-where-stmt -> END WHERE [where-construct-name]
WRAPPER_CLASS(EndWhereStmt, std::optional<Name>);

// R1042 where-construct ->
//         where-construct-stmt [where-body-construct]...
//         [masked-elsewhere-stmt [where-body-construct]...]...
//         [elsewhere-stmt [where-body-construct]...] end-where-stmt
struct WhereConstruct {
  struct MaskedElsewhere {
    TUPLE_CLASS_BOILERPLATE(MaskedElsewhere);
    std::tuple<Statement<MaskedElsewhereStmt>, std::list<WhereBodyConstruct>> t;
  };
  struct Elsewhere {
    TUPLE_CLASS_BOILERPLATE(Elsewhere);
    std::tuple<Statement<ElsewhereStmt>, std::list<WhereBodyConstruct>> t;
  };
  TUPLE_CLASS_BOILERPLATE(WhereConstruct);
  std::tuple<Statement<WhereConstructStmt>, std::list<WhereBodyConstruct>,
      std::list<MaskedElsewhere>, std::optional<Elsewhere>,
      Statement<EndWhereStmt>>
      t;
};

// R1051 forall-construct-stmt ->
//         [forall-construct-name :] FORALL concurrent-header
struct ForallConstructStmt {
  TUPLE_CLASS_BOILERPLATE(ForallConstructStmt);
  std::tuple<std::optional<Name>, Indirection<ConcurrentHeader>> t;
};

// R1053 forall-assignment-stmt -> assignment-stmt | pointer-assignment-stmt
struct ForallAssignmentStmt {
  UNION_CLASS_BOILERPLATE(ForallAssignmentStmt);
  std::variant<AssignmentStmt, PointerAssignmentStmt> u;
};

// R1055 forall-stmt -> FORALL concurrent-header forall-assignment-stmt
struct ForallStmt {
  TUPLE_CLASS_BOILERPLATE(ForallStmt);
  std::tuple<Indirection<ConcurrentHeader>, ForallAssignmentStmt> t;
};

// R1052 forall-body-construct ->
//         forall-assignment-stmt | where-stmt | where-construct |
//         forall-construct | forall-stmt
struct ForallBodyConstruct {
  UNION_CLASS_BOILERPLATE(ForallBodyConstruct);
  std::variant<Statement<ForallAssignmentStmt>, Statement<WhereStmt>,
      WhereConstruct, Indirection<ForallConstruct>, Statement<ForallStmt>>
      u;
};

// R1054 end-forall-stmt -> END FORALL [forall-construct-name]
WRAPPER_CLASS(EndForallStmt, std::optional<Name>);

// R1050 forall-construct ->
//         forall-construct-stmt [forall-body-construct]... end-forall-stmt
struct ForallConstruct {
  TUPLE_CLASS_BOILERPLATE(ForallConstruct);
  std::tuple<Statement<ForallConstructStmt>, std::list<ForallBodyConstruct>,
      Statement<EndForallStmt>>
      t;
};

// R1101 block -> [execution-part-construct]...
using Block = std::list<ExecutionPartConstruct>;

// R1105 selector -> expr | variable
struct Selector {
  UNION_CLASS_BOILERPLATE(Selector);
  std::variant<Expr, Variable> u;
};

// R1104 association -> associate-name => selector
struct Association {
  TUPLE_CLASS_BOILERPLATE(Association);
  std::tuple<Name, Selector> t;
};

// R1103 associate-stmt ->
//        [associate-construct-name :] ASSOCIATE ( association-list )
struct AssociateStmt {
  TUPLE_CLASS_BOILERPLATE(AssociateStmt);
  std::tuple<std::optional<Name>, std::list<Association>> t;
};

// R1106 end-associate-stmt -> END ASSOCIATE [associate-construct-name]
WRAPPER_CLASS(EndAssociateStmt, std::optional<Name>);

// R1102 associate-construct -> associate-stmt block end-associate-stmt
struct AssociateConstruct {
  TUPLE_CLASS_BOILERPLATE(AssociateConstruct);
  std::tuple<Statement<AssociateStmt>, Block, Statement<EndAssociateStmt>> t;
};

// R1108 block-stmt -> [block-construct-name :] BLOCK
WRAPPER_CLASS(BlockStmt, std::optional<Name>);

// R1110 end-block-stmt -> END BLOCK [block-construct-name]
WRAPPER_CLASS(EndBlockStmt, std::optional<Name>);

// R1109 block-specification-part ->
//         [use-stmt]... [import-stmt]... [implicit-part]
//         [[declaration-construct]... specification-construct]
WRAPPER_CLASS(BlockSpecificationPart, SpecificationPart);
// TODO: error if any COMMON, EQUIVALENCE, INTENT, NAMELIST, OPTIONAL,
// VALUE, ENTRY, SAVE /common/, or statement function definition statement
// appears in a block-specification part (C1107, C1570).

// R1107 block-construct ->
//         block-stmt [block-specification-part] block end-block-stmt
struct BlockConstruct {
  TUPLE_CLASS_BOILERPLATE(BlockConstruct);
  std::tuple<Statement<BlockStmt>, BlockSpecificationPart, Block,
      Statement<EndBlockStmt>>
      t;
};

// R1113 coarray-association -> codimension-decl => selector
struct CoarrayAssociation {
  TUPLE_CLASS_BOILERPLATE(CoarrayAssociation);
  std::tuple<CodimensionDecl, Selector> t;
};

// R1112 change-team-stmt ->
//         [team-construct-name :] CHANGE TEAM
//         ( team-variable [, coarray-association-list] [, sync-stat-list] )
struct ChangeTeamStmt {
  TUPLE_CLASS_BOILERPLATE(ChangeTeamStmt);
  std::tuple<std::optional<Name>, TeamVariable, std::list<CoarrayAssociation>,
      std::list<StatOrErrmsg>>
      t;
};

// R1114 end-change-team-stmt ->
//         END TEAM [( [sync-stat-list] )] [team-construct-name]
struct EndChangeTeamStmt {
  TUPLE_CLASS_BOILERPLATE(EndChangeTeamStmt);
  std::tuple<std::list<StatOrErrmsg>, std::optional<Name>> t;
};

// R1111 change-team-construct -> change-team-stmt block end-change-team-stmt
struct ChangeTeamConstruct {
  TUPLE_CLASS_BOILERPLATE(ChangeTeamConstruct);
  std::tuple<Statement<ChangeTeamStmt>, Block, Statement<EndChangeTeamStmt>> t;
};

// R1117 critical-stmt ->
//         [critical-construct-name :] CRITICAL [( [sync-stat-list] )]
struct CriticalStmt {
  TUPLE_CLASS_BOILERPLATE(CriticalStmt);
  std::tuple<std::optional<Name>, std::list<StatOrErrmsg>> t;
};

// R1118 end-critical-stmt -> END CRITICAL [critical-construct-name]
WRAPPER_CLASS(EndCriticalStmt, std::optional<Name>);

// R1116 critical-construct -> critical-stmt block end-critical-stmt
struct CriticalConstruct {
  TUPLE_CLASS_BOILERPLATE(CriticalConstruct);
  std::tuple<Statement<CriticalStmt>, Block, Statement<EndCriticalStmt>> t;
};

// R1126 concurrent-control ->
//         index-name = concurrent-limit : concurrent-limit [: concurrent-step]
// R1127 concurrent-limit -> scalar-int-expr
// R1128 concurrent-step -> scalar-int-expr
struct ConcurrentControl {
  TUPLE_CLASS_BOILERPLATE(ConcurrentControl);
  std::tuple<Name, ScalarIntExpr, ScalarIntExpr, std::optional<ScalarIntExpr>>
      t;
};

// R1125 concurrent-header ->
//         ( [integer-type-spec ::] concurrent-control-list
//         [, scalar-mask-expr] )
struct ConcurrentHeader {
  TUPLE_CLASS_BOILERPLATE(ConcurrentHeader);
  std::tuple<std::optional<IntegerTypeSpec>, std::list<ConcurrentControl>,
      std::optional<ScalarLogicalExpr>>
      t;
};

// R1130 locality-spec ->
//         LOCAL ( variable-name-list ) | LOCAL INIT ( variable-name-list ) |
//         SHARED ( variable-name-list ) | DEFAULT ( NONE )
struct LocalitySpec {
  UNION_CLASS_BOILERPLATE(LocalitySpec);
  WRAPPER_CLASS(Local, std::list<Name>);
  WRAPPER_CLASS(LocalInit, std::list<Name>);
  WRAPPER_CLASS(Shared, std::list<Name>);
  EMPTY_CLASS(DefaultNone);
  std::variant<Local, LocalInit, Shared, DefaultNone> u;
};

// R1123 loop-control ->
//         [,] do-variable = scalar-int-expr , scalar-int-expr
//           [, scalar-int-expr] |
//         [,] WHILE ( scalar-logical-expr ) |
//         [,] CONCURRENT concurrent-header concurrent-locality
// R1129 concurrent-locality -> [locality-spec]...
struct LoopControl {
  UNION_CLASS_BOILERPLATE(LoopControl);
  struct Concurrent {
    TUPLE_CLASS_BOILERPLATE(Concurrent);
    std::tuple<ConcurrentHeader, std::list<LocalitySpec>> t;
  };
  std::variant<LoopBounds<ScalarIntExpr>, ScalarLogicalExpr, Concurrent> u;
};

// R1121 label-do-stmt -> [do-construct-name :] DO label [loop-control]
struct LabelDoStmt {
  TUPLE_CLASS_BOILERPLATE(LabelDoStmt);
  std::tuple<std::optional<Name>, Label, std::optional<LoopControl>> t;
};

// R1122 nonlabel-do-stmt -> [do-construct-name :] DO [loop-control]
struct NonLabelDoStmt {
  TUPLE_CLASS_BOILERPLATE(NonLabelDoStmt);
  std::tuple<std::optional<Name>, std::optional<LoopControl>> t;
};

// R1132 end-do-stmt -> END DO [do-construct-name]
WRAPPER_CLASS(EndDoStmt, std::optional<Name>);

// R1131 end-do -> end-do-stmt | continue-stmt

// R1119 do-construct -> do-stmt block end-do
// R1120 do-stmt -> nonlabel-do-stmt | label-do-stmt
// TODO: deprecated: DO loop ending on statement types other than END DO and
// CONTINUE; multiple "label DO" loops ending on the same label
struct DoConstruct {
  TUPLE_CLASS_BOILERPLATE(DoConstruct);
  std::tuple<Statement<NonLabelDoStmt>, Block, Statement<EndDoStmt>> t;
};

// R1133 cycle-stmt -> CYCLE [do-construct-name]
WRAPPER_CLASS(CycleStmt, std::optional<Name>);

// R1135 if-then-stmt -> [if-construct-name :] IF ( scalar-logical-expr ) THEN
struct IfThenStmt {
  TUPLE_CLASS_BOILERPLATE(IfThenStmt);
  std::tuple<std::optional<Name>, ScalarLogicalExpr> t;
};

// R1136 else-if-stmt ->
//         ELSE IF ( scalar-logical-expr ) THEN [if-construct-name]
struct ElseIfStmt {
  TUPLE_CLASS_BOILERPLATE(ElseIfStmt);
  std::tuple<ScalarLogicalExpr, std::optional<Name>> t;
};

// R1137 else-stmt -> ELSE [if-construct-name]
WRAPPER_CLASS(ElseStmt, std::optional<Name>);

// R1138 end-if-stmt -> END IF [if-construct-name]
WRAPPER_CLASS(EndIfStmt, std::optional<Name>);

// R1134 if-construct ->
//         if-then-stmt block [else-if-stmt block]...
//         [else-stmt block] end-if-stmt
struct IfConstruct {
  struct ElseIfBlock {
    TUPLE_CLASS_BOILERPLATE(ElseIfBlock);
    std::tuple<Statement<ElseIfStmt>, Block> t;
  };
  struct ElseBlock {
    TUPLE_CLASS_BOILERPLATE(ElseBlock);
    std::tuple<Statement<ElseStmt>, Block> t;
  };
  TUPLE_CLASS_BOILERPLATE(IfConstruct);
  std::tuple<Statement<IfThenStmt>, Block, std::list<ElseIfBlock>,
      std::optional<ElseBlock>, Statement<EndIfStmt>>
      t;
};

// R1139 if-stmt -> IF ( scalar-logical-expr ) action-stmt
struct IfStmt {
  TUPLE_CLASS_BOILERPLATE(IfStmt);
  std::tuple<ScalarLogicalExpr, ActionStmt> t;
};

// R1141 select-case-stmt -> [case-construct-name :] SELECT CASE ( case-expr )
// R1144 case-expr -> scalar-expr
struct SelectCaseStmt {
  TUPLE_CLASS_BOILERPLATE(SelectCaseStmt);
  std::tuple<std::optional<Name>, Scalar<Expr>> t;
};

// R1147 case-value -> scalar-constant-expr
using CaseValue = Scalar<ConstantExpr>;

// R1146 case-value-range ->
//         case-value | case-value : | : case-value | case-value : case-value
struct CaseValueRange {
  UNION_CLASS_BOILERPLATE(CaseValueRange);
  struct Range {
    BOILERPLATE(Range);
    Range(std::optional<CaseValue> &&l, std::optional<CaseValue> &&u)
      : lower{std::move(l)}, upper{std::move(u)} {}
    std::optional<CaseValue> lower, upper;  // not both missing
  };
  std::variant<CaseValue, Range> u;
};

// R1145 case-selector -> ( case-value-range-list ) | DEFAULT
EMPTY_CLASS(Default);

struct CaseSelector {
  UNION_CLASS_BOILERPLATE(CaseSelector);
  std::variant<std::list<CaseValueRange>, Default> u;
};

// R1142 case-stmt -> CASE case-selector [case-construct-name]
struct CaseStmt {
  TUPLE_CLASS_BOILERPLATE(CaseStmt);
  std::tuple<CaseSelector, std::optional<Name>> t;
};

// R1143 end-select-stmt -> END SELECT [case-construct-name]
// R1151 end-select-rank-stmt -> END SELECT [select-construct-name]
// R1155 end-select-type-stmt -> END SELECT [select-construct-name]
WRAPPER_CLASS(EndSelectStmt, std::optional<Name>);

// R1140 case-construct ->
//         select-case-stmt [case-stmt block]... end-select-stmt
struct CaseConstruct {
  struct Case {
    TUPLE_CLASS_BOILERPLATE(Case);
    std::tuple<Statement<CaseStmt>, Block> t;
  };
  TUPLE_CLASS_BOILERPLATE(CaseConstruct);
  std::tuple<Statement<SelectCaseStmt>, std::list<Case>,
      Statement<EndSelectStmt>>
      t;
};

// R1149 select-rank-stmt ->
//         [select-construct-name :] SELECT RANK
//         ( [associate-name =>] selector )
struct SelectRankStmt {
  TUPLE_CLASS_BOILERPLATE(SelectRankStmt);
  std::tuple<std::optional<Name>, std::optional<Name>, Selector> t;
};

// R1150 select-rank-case-stmt ->
//         RANK ( scalar-int-constant-expr ) [select-construct-name] |
//         RANK ( * ) [select-construct-name] |
//         RANK DEFAULT [select-construct-name]
struct SelectRankCaseStmt {
  struct Rank {
    UNION_CLASS_BOILERPLATE(Rank);
    std::variant<ScalarIntConstantExpr, Star, Default> u;
  };
  TUPLE_CLASS_BOILERPLATE(SelectRankCaseStmt);
  std::tuple<Rank, std::optional<Name>> t;
};

// R1148 select-rank-construct ->
//         select-rank-stmt [select-rank-case-stmt block]...
//         end-select-rank-stmt
struct SelectRankConstruct {
  TUPLE_CLASS_BOILERPLATE(SelectRankConstruct);
  struct RankCase {
    TUPLE_CLASS_BOILERPLATE(RankCase);
    std::tuple<Statement<SelectRankCaseStmt>, Block> t;
  };
  std::tuple<Statement<SelectRankStmt>, std::list<RankCase>,
      Statement<EndSelectStmt>>
      t;
};

// R1153 select-type-stmt ->
//         [select-construct-name :] SELECT TYPE
//         ( [associate-name =>] selector )
struct SelectTypeStmt {
  TUPLE_CLASS_BOILERPLATE(SelectTypeStmt);
  std::tuple<std::optional<Name>, std::optional<Name>, Selector> t;
};

// R1154 type-guard-stmt ->
//         TYPE IS ( type-spec ) [select-construct-name] |
//         CLASS IS ( derived-type-spec ) [select-construct-name] |
//         CLASS DEFAULT [select-construct-name]
struct TypeGuardStmt {
  struct Guard {
    UNION_CLASS_BOILERPLATE(Guard);
    std::variant<TypeSpec, DerivedTypeSpec, Default> u;
  };
  TUPLE_CLASS_BOILERPLATE(TypeGuardStmt);
  std::tuple<Guard, std::optional<Name>> t;
};

// R1152 select-type-construct ->
//         select-type-stmt [type-guard-stmt block]... end-select-type-stmt
struct SelectTypeConstruct {
  TUPLE_CLASS_BOILERPLATE(SelectTypeConstruct);
  struct TypeCase {
    TUPLE_CLASS_BOILERPLATE(TypeCase);
    std::tuple<Statement<TypeGuardStmt>, Block> t;
  };
  std::tuple<Statement<SelectTypeStmt>, std::list<TypeCase>,
      Statement<EndSelectStmt>>
      t;
};

// R1156 exit-stmt -> EXIT [construct-name]
WRAPPER_CLASS(ExitStmt, std::optional<Name>);

// R1157 goto-stmt -> GO TO label
WRAPPER_CLASS(GotoStmt, Label);

// R1158 computed-goto-stmt -> GO TO ( label-list ) [,] scalar-int-expr
struct ComputedGotoStmt {
  TUPLE_CLASS_BOILERPLATE(ComputedGotoStmt);
  std::tuple<std::list<Label>, ScalarIntExpr> t;
};

// R1162 stop-code -> scalar-default-char-expr | scalar-int-expr
struct StopCode {
  UNION_CLASS_BOILERPLATE(StopCode);
  std::variant<ScalarDefaultCharExpr, ScalarIntExpr> u;
};

// R1160 stop-stmt -> STOP [stop-code] [, QUIET = scalar-logical-expr]
// R1161 error-stop-stmt ->
//         ERROR STOP [stop-code] [, QUIET = scalar-logical-expr]
struct StopStmt {
  enum class Kind { Stop, ErrorStop };
  TUPLE_CLASS_BOILERPLATE(StopStmt);
  std::tuple<Kind, std::optional<StopCode>, std::optional<ScalarLogicalExpr>> t;
};

// R1164 sync-all-stmt -> SYNC ALL [( [sync-stat-list] )]
WRAPPER_CLASS(SyncAllStmt, std::list<StatOrErrmsg>);

// R1166 sync-images-stmt -> SYNC IMAGES ( image-set [, sync-stat-list] )
// R1167 image-set -> int-expr | *
struct SyncImagesStmt {
  struct ImageSet {
    UNION_CLASS_BOILERPLATE(ImageSet);
    std::variant<IntExpr, Star> u;
  };
  TUPLE_CLASS_BOILERPLATE(SyncImagesStmt);
  std::tuple<ImageSet, std::list<StatOrErrmsg>> t;
};

// R1168 sync-memory-stmt -> SYNC MEMORY [( [sync-stat-list] )]
WRAPPER_CLASS(SyncMemoryStmt, std::list<StatOrErrmsg>);

// R1169 sync-team-stmt -> SYNC TEAM ( team-variable [, sync-stat-list] )
struct SyncTeamStmt {
  TUPLE_CLASS_BOILERPLATE(SyncTeamStmt);
  std::tuple<TeamVariable, std::list<StatOrErrmsg>> t;
};

// R1171 event-variable -> scalar-variable
using EventVariable = Scalar<Variable>;

// R1170 event-post-stmt -> EVENT POST ( event-variable [, sync-stat-list] )
struct EventPostStmt {
  TUPLE_CLASS_BOILERPLATE(EventPostStmt);
  std::tuple<EventVariable, std::list<StatOrErrmsg>> t;
};

// R1172 event-wait-stmt ->
//         EVENT WAIT ( event-variable [, event-wait-spec-list] )
// R1173 event-wait-spec -> until-spec | sync-stat
// R1174 until-spec -> UNTIL_COUNT = scalar-int-expr
struct EventWaitStmt {
  struct EventWaitSpec {
    UNION_CLASS_BOILERPLATE(EventWaitSpec);
    std::variant<ScalarIntExpr, StatOrErrmsg> u;
  };
  TUPLE_CLASS_BOILERPLATE(EventWaitStmt);
  std::tuple<EventVariable, std::list<EventWaitSpec>> t;
};

// R1175 form-team-stmt ->
//         FORM TEAM ( team-number , team-variable [, form-team-spec-list] )
// R1176 team-number -> scalar-int-expr
// R1177 form-team-spec -> NEW_INDEX = scalar-int-expr | sync-stat
struct FormTeamStmt {
  struct FormTeamSpec {
    UNION_CLASS_BOILERPLATE(FormTeamSpec);
    std::variant<ScalarIntExpr, StatOrErrmsg> u;
  };
  TUPLE_CLASS_BOILERPLATE(FormTeamStmt);
  std::tuple<ScalarIntExpr, TeamVariable, std::list<FormTeamSpec>> t;
};

// R1181 lock-variable -> scalar-variable
using LockVariable = Scalar<Variable>;

// R1178 lock-stmt -> LOCK ( lock-variable [, lock-stat-list] )
// R1179 lock-stat -> ACQUIRED_LOCK = scalar-logical-variable | sync-stat
struct LockStmt {
  struct LockStat {
    UNION_CLASS_BOILERPLATE(LockStat);
    std::variant<Scalar<Logical<Variable>>, StatOrErrmsg> u;
  };
  TUPLE_CLASS_BOILERPLATE(LockStmt);
  std::tuple<LockVariable, std::list<LockStat>> t;
};

// R1180 unlock-stmt -> UNLOCK ( lock-variable [, sync-stat-list] )
struct UnlockStmt {
  TUPLE_CLASS_BOILERPLATE(UnlockStmt);
  std::tuple<LockVariable, std::list<StatOrErrmsg>> t;
};

// R1202 file-unit-number -> scalar-int-expr
WRAPPER_CLASS(FileUnitNumber, ScalarIntExpr);

// R1201 io-unit -> file-unit-number | * | internal-file-variable
// R1203 internal-file-variable -> char-variable
struct IoUnit {
  UNION_CLASS_BOILERPLATE(IoUnit);
  std::variant<FileUnitNumber, Star, CharVariable> u;
};

// R1206 file-name-expr -> scalar-default-char-expr
using FileNameExpr = ScalarDefaultCharExpr;

// R1205 connect-spec ->
//         [UNIT =] file-unit-number | ACCESS = scalar-default-char-expr |
//         ACTION = scalar-default-char-expr |
//         ASYNCHRONOUS = scalar-default-char-expr |
//         BLANK = scalar-default-char-expr |
//         DECIMAL = scalar-default-char-expr |
//         DELIM = scalar-default-char-expr |
//         ENCODING = scalar-default-char-expr | ERR = label |
//         FILE = file-name-expr | FORM = scalar-default-char-expr |
//         IOMSG = iomsg-variable | IOSTAT = scalar-int-variable |
//         NEWUNIT = scalar-int-variable | PAD = scalar-default-char-expr |
//         POSITION = scalar-default-char-expr | RECL = scalar-int-expr |
//         ROUND = scalar-default-char-expr | SIGN = scalar-default-char-expr |
//         STATUS = scalar-default-char-expr
WRAPPER_CLASS(StatusExpr, ScalarDefaultCharExpr);
WRAPPER_CLASS(ErrLabel, Label);

struct ConnectSpec {
  UNION_CLASS_BOILERPLATE(ConnectSpec);
  struct CharExpr {
    enum class Kind {
      Access,
      Action,
      Asynchronous,
      Blank,
      Decimal,
      Delim,
      Encoding,
      Form,
      Pad,
      Position,
      Round,
      Sign,
      Dispose /*extension*/
    };
    TUPLE_CLASS_BOILERPLATE(CharExpr);
    std::tuple<Kind, ScalarDefaultCharExpr> t;
  };
  WRAPPER_CLASS(Recl, ScalarIntExpr);
  WRAPPER_CLASS(Newunit, ScalarIntVariable);
  std::variant<FileUnitNumber, FileNameExpr, CharExpr, StatVariable, Recl,
      Newunit, ErrLabel, StatusExpr>
      u;
};

// R1204 open-stmt -> OPEN ( connect-spec-list )
WRAPPER_CLASS(OpenStmt, std::list<ConnectSpec>);

// R1208 close-stmt -> CLOSE ( close-spec-list )
// R1209 close-spec ->
//         [UNIT =] file-unit-number | IOSTAT = scalar-int-variable |
//         IOMSG = iomsg-variable | ERR = label |
//         STATUS = scalar-default-char-expr
struct CloseStmt {
  struct CloseSpec {
    UNION_CLASS_BOILERPLATE(CloseSpec);
    std::variant<FileUnitNumber, StatVariable, MsgVariable, ErrLabel,
        StatusExpr>
        u;
  };
  WRAPPER_CLASS_BOILERPLATE(CloseStmt, std::list<CloseSpec>);
};

// R1215 format -> default-char-expr | label | *
struct Format {
  UNION_CLASS_BOILERPLATE(Format);
  std::variant<DefaultCharExpr, Label, Star> u;
};

// R1214 id-variable -> scalar-int-variable
WRAPPER_CLASS(IdVariable, ScalarIntVariable);

// R1213 io-control-spec ->
//         [UNIT =] io-unit | [FMT =] format | [NML =] namelist-group-name |
//         ADVANCE = scalar-default-char-expr |
//         ASYNCHRONOUS = scalar-default-char-constant-expr |
//         BLANK = scalar-default-char-expr |
//         DECIMAL = scalar-default-char-expr |
//         DELIM = scalar-default-char-expr | END = label | EOR = label |
//         ERR = label | ID = id-variable | IOMSG = iomsg-variable |
//         IOSTAT = scalar-int-variable | PAD = scalar-default-char-expr |
//         POS = scalar-int-expr | REC = scalar-int-expr |
//         ROUND = scalar-default-char-expr | SIGN = scalar-default-char-expr |
//         SIZE = scalar-int-variable
WRAPPER_CLASS(EndLabel, Label);
WRAPPER_CLASS(EorLabel, Label);
struct IoControlSpec {
  UNION_CLASS_BOILERPLATE(IoControlSpec);
  struct CharExpr {
    enum class Kind { Advance, Blank, Decimal, Delim, Pad, Round, Sign };
    TUPLE_CLASS_BOILERPLATE(CharExpr);
    std::tuple<Kind, ScalarDefaultCharExpr> t;
  };
  WRAPPER_CLASS(Asynchronous, ScalarDefaultCharConstantExpr);
  WRAPPER_CLASS(Pos, ScalarIntExpr);
  WRAPPER_CLASS(Rec, ScalarIntExpr);
  WRAPPER_CLASS(Size, ScalarIntVariable);
  std::variant<IoUnit, Format, Name, CharExpr, Asynchronous, EndLabel, EorLabel,
      ErrLabel, IdVariable, MsgVariable, StatVariable, Pos, Rec, Size>
      u;
};

// R1216 input-item -> variable | io-implied-do
struct InputItem {
  UNION_CLASS_BOILERPLATE(InputItem);
  std::variant<Variable, Indirection<InputImpliedDo>> u;
};

// R1210 read-stmt ->
//         READ ( io-control-spec-list ) [input-item-list] |
//         READ format [, input-item-list]
struct ReadStmt {
  BOILERPLATE(ReadStmt);
  ReadStmt(std::optional<IoUnit> &&i, std::optional<Format> &&f,
      std::list<IoControlSpec> &&cs, std::list<InputItem> &&its)
    : iounit{std::move(i)}, format{std::move(f)}, controls(std::move(cs)),
      items(std::move(its)) {}
  std::optional<IoUnit> iounit;  // if first in controls without UNIT=
  std::optional<Format> format;  // if second in controls without FMT=, or
                                 // no (io-control-spec-list); might be
                                 // an untagged namelist group name (TODO)
  std::list<IoControlSpec> controls;
  std::list<InputItem> items;
};

// R1217 output-item -> expr | io-implied-do
struct OutputItem {
  UNION_CLASS_BOILERPLATE(OutputItem);
  std::variant<Expr, Indirection<OutputImpliedDo>> u;
};

// R1211 write-stmt -> WRITE ( io-control-spec-list ) [output-item-list]
struct WriteStmt {
  BOILERPLATE(WriteStmt);
  WriteStmt(std::optional<IoUnit> &&i, std::optional<Format> &&f,
      std::list<IoControlSpec> &&cs, std::list<OutputItem> &&its)
    : iounit{std::move(i)}, format{std::move(f)}, controls(std::move(cs)),
      items(std::move(its)) {}
  std::optional<IoUnit> iounit;  // if first in controls without UNIT=
  std::optional<Format> format;  // if second in controls without FMT=;
                                 // might be an untagged namelist group (TODO)
  std::list<IoControlSpec> controls;
  std::list<OutputItem> items;
};

// R1212 print-stmt PRINT format [, output-item-list]
struct PrintStmt {
  TUPLE_CLASS_BOILERPLATE(PrintStmt);
  std::tuple<Format, std::list<OutputItem>> t;
};

// R1220 io-implied-do-control ->
//         do-variable = scalar-int-expr , scalar-int-expr [, scalar-int-expr]
using IoImpliedDoControl = LoopBounds<ScalarIntExpr>;

// R1218 io-implied-do -> ( io-implied-do-object-list , io-implied-do-control )
// R1219 io-implied-do-object -> input-item | output-item
struct InputImpliedDo {
  TUPLE_CLASS_BOILERPLATE(InputImpliedDo);
  std::tuple<std::list<InputItem>, IoImpliedDoControl> t;
};

struct OutputImpliedDo {
  TUPLE_CLASS_BOILERPLATE(OutputImpliedDo);
  std::tuple<std::list<OutputItem>, IoImpliedDoControl> t;
};

// R1223 wait-spec ->
//         [UNIT =] file-unit-number | END = label | EOR = label | ERR = label |
//         ID = scalar-int-expr | IOMSG = iomsg-variable |
//         IOSTAT = scalar-int-variable
WRAPPER_CLASS(IdExpr, ScalarIntExpr);
struct WaitSpec {
  UNION_CLASS_BOILERPLATE(WaitSpec);
  std::variant<FileUnitNumber, EndLabel, EorLabel, ErrLabel, IdExpr,
      MsgVariable, StatVariable>
      u;
};

// R1222 wait-stmt -> WAIT ( wait-spec-list )
WRAPPER_CLASS(WaitStmt, std::list<WaitSpec>);

// R1227 position-spec ->
//         [UNIT =] file-unit-number | IOMSG = iomsg-variable |
//         IOSTAT = scalar-int-variable | ERR = label
// R1229 flush-spec ->
//         [UNIT =] file-unit-number | IOSTAT = scalar-int-variable |
//         IOMSG = iomsg-variable | ERR = label
struct PositionOrFlushSpec {
  UNION_CLASS_BOILERPLATE(PositionOrFlushSpec);
  std::variant<FileUnitNumber, MsgVariable, StatVariable, ErrLabel> u;
};

// R1224 backspace-stmt ->
//         BACKSPACE file-unit-number | BACKSPACE ( position-spec-list )
struct BackspaceStmt {
  UNION_CLASS_BOILERPLATE(BackspaceStmt);
  std::variant<FileUnitNumber, std::list<PositionOrFlushSpec>> u;
};

// R1225 endfile-stmt ->
//         ENDFILE file-unit-number | ENDFILE ( position-spec-list )
struct EndfileStmt {
  UNION_CLASS_BOILERPLATE(EndfileStmt);
  std::variant<FileUnitNumber, std::list<PositionOrFlushSpec>> u;
};

// R1226 rewind-stmt -> REWIND file-unit-number | REWIND ( position-spec-list )
struct RewindStmt {
  UNION_CLASS_BOILERPLATE(RewindStmt);
  std::variant<FileUnitNumber, std::list<PositionOrFlushSpec>> u;
};

// R1228 flush-stmt -> FLUSH file-unit-number | FLUSH ( flush-spec-list )
struct FlushStmt {
  UNION_CLASS_BOILERPLATE(FlushStmt);
  std::variant<FileUnitNumber, std::list<PositionOrFlushSpec>> u;
};

// R1231 inquire-spec ->
//         [UNIT =] file-unit-number | FILE = file-name-expr |
//         ACCESS = scalar-default-char-variable |
//         ACTION = scalar-default-char-variable |
//         ASYNCHRONOUS = scalar-default-char-variable |
//         BLANK = scalar-default-char-variable |
//         DECIMAL = scalar-default-char-variable |
//         DELIM = scalar-default-char-variable |
//         ENCODING = scalar-default-char-variable |
//         ERR = label | EXIST = scalar-logical-variable |
//         FORM = scalar-default-char-variable |
//         FORMATTED = scalar-default-char-variable |
//         ID = scalar-int-expr | IOMSG = iomsg-variable |
//         IOSTAT = scalar-int-variable |
//         NAME = scalar-default-char-variable |
//         NAMED = scalar-logical-variable |
//         NEXTREC = scalar-int-variable | NUMBER = scalar-int-variable |
//         OPENED = scalar-logical-variable |
//         PAD = scalar-default-char-variable |
//         PENDING = scalar-logical-variable | POS = scalar-int-variable |
//         POSITION = scalar-default-char-variable |
//         READ = scalar-default-char-variable |
//         READWRITE = scalar-default-char-variable |
//         RECL = scalar-int-variable | ROUND = scalar-default-char-variable |
//         SEQUENTIAL = scalar-default-char-variable |
//         SIGN = scalar-default-char-variable |
//         SIZE = scalar-int-variable |
//         STREAM = scalar-default-char-variable |
//         STATUS = scalar-default-char-variable |
//         WRITE = scalar-default-char-variable
struct InquireSpec {
  UNION_CLASS_BOILERPLATE(InquireSpec);
  struct CharVar {
    enum class Kind {
      Access,
      Action,
      Asynchronous,
      Blank,
      Decimal,
      Delim,
      Encoding,
      Form,
      Formatted,
      Iomsg,
      Name,
      Pad,
      Position,
      Read,
      Readwrite,
      Round,
      Sequential,
      Sign,
      Stream,
      Status,
      Write
    };
    TUPLE_CLASS_BOILERPLATE(CharVar);
    std::tuple<Kind, ScalarDefaultCharVariable> t;
  };
  struct IntVar {
    enum class Kind { Iostat, Nextrec, Number, Pos, Recl, Size };
    TUPLE_CLASS_BOILERPLATE(IntVar);
    std::tuple<Kind, ScalarIntVariable> t;
  };
  struct LogVar {
    enum class Kind { Exist, Named, Opened, Pending };
    TUPLE_CLASS_BOILERPLATE(LogVar);
    std::tuple<Kind, Scalar<Logical<Variable>>> t;
  };
  std::variant<FileUnitNumber, FileNameExpr, CharVar, IntVar, LogVar, IdExpr,
      ErrLabel>
      u;
};

// R1230 inquire-stmt ->
//         INQUIRE ( inquire-spec-list ) |
//         INQUIRE ( IOLENGTH = scalar-int-variable ) output-item-list
struct InquireStmt {
  UNION_CLASS_BOILERPLATE(InquireStmt);
  struct Iolength {
    TUPLE_CLASS_BOILERPLATE(Iolength);
    std::tuple<ScalarIntVariable, std::list<OutputItem>> t;
  };
  std::variant<std::list<InquireSpec>, Iolength> u;
};

// R1301 format-stmt -> FORMAT format-specification
WRAPPER_CLASS(FormatStmt, FormatSpecification);

// R1403 end-program-stmt -> END [PROGRAM [program-name]]
WRAPPER_CLASS(EndProgramStmt, std::optional<Name>);

// R1401 main-program ->
//         [program-stmt] [specification-part] [execution-part]
//         [internal-subprogram-part] end-program-stmt
// R1402 program-stmt -> PROGRAM program-name
struct MainProgram {
  TUPLE_CLASS_BOILERPLATE(MainProgram);
  std::tuple<std::optional<Statement<Name>>, SpecificationPart, ExecutionPart,
      std::optional<InternalSubprogramPart>, Statement<EndProgramStmt>>
      t;
};

// R1405 module-stmt -> MODULE module-name
WRAPPER_CLASS(ModuleStmt, Name);

// R1408 module-subprogram ->
//         function-subprogram | subroutine-subprogram |
//         separate-module-subprogram
struct ModuleSubprogram {
  UNION_CLASS_BOILERPLATE(ModuleSubprogram);
  std::variant<Indirection<FunctionSubprogram>,
      Indirection<SubroutineSubprogram>, Indirection<SeparateModuleSubprogram>>
      u;
};

// R1407 module-subprogram-part -> contains-stmt [module-subprogram]...
struct ModuleSubprogramPart {
  TUPLE_CLASS_BOILERPLATE(ModuleSubprogramPart);
  std::tuple<Statement<ContainsStmt>, std::list<ModuleSubprogram>> t;
};

// R1406 end-module-stmt -> END [MODULE [module-name]]
WRAPPER_CLASS(EndModuleStmt, std::optional<Name>);

// R1404 module ->
//         module-stmt [specification-part] [module-subprogram-part]
//         end-module-stmt
struct Module {
  TUPLE_CLASS_BOILERPLATE(Module);
  std::tuple<Statement<ModuleStmt>, SpecificationPart,
      std::optional<ModuleSubprogramPart>, Statement<EndModuleStmt>>
      t;
};

// R1411 rename ->
//         local-name => use-name |
//         OPERATOR ( local-defined-operator ) =>
//           OPERATOR ( use-defined-operator )
struct Rename {
  UNION_CLASS_BOILERPLATE(Rename);
  struct Names {
    TUPLE_CLASS_BOILERPLATE(Names);
    std::tuple<Name, Name> t;
  };
  struct Operators {
    TUPLE_CLASS_BOILERPLATE(Operators);
    std::tuple<DefinedOpName, DefinedOpName> t;
  };
  std::variant<Names, Operators> u;
};

// R1418 parent-identifier -> ancestor-module-name [: parent-submodule-name]
struct ParentIdentifier {
  TUPLE_CLASS_BOILERPLATE(ParentIdentifier);
  std::tuple<Name, std::optional<Name>> t;
};

// R1417 submodule-stmt -> SUBMODULE ( parent-identifier ) submodule-name
struct SubmoduleStmt {
  TUPLE_CLASS_BOILERPLATE(SubmoduleStmt);
  std::tuple<ParentIdentifier, Name> t;
};

// R1419 end-submodule-stmt -> END [SUBMODULE [submodule-name]]
WRAPPER_CLASS(EndSubmoduleStmt, std::optional<Name>);

// R1416 submodule ->
//         submodule-stmt [specification-part] [module-subprogram-part]
//         end-submodule-stmt
struct Submodule {
  TUPLE_CLASS_BOILERPLATE(Submodule);
  std::tuple<Statement<SubmoduleStmt>, SpecificationPart,
      std::optional<ModuleSubprogramPart>, Statement<EndSubmoduleStmt>>
      t;
};

// R1421 block-data-stmt -> BLOCK DATA [block-data-name]
WRAPPER_CLASS(BlockDataStmt, std::optional<Name>);

// R1422 end-block-data-stmt -> END [BLOCK DATA [block-data-name]]
WRAPPER_CLASS(EndBlockDataStmt, std::optional<Name>);

// R1420 block-data -> block-data-stmt [specification-part] end-block-data-stmt
struct BlockData {
  TUPLE_CLASS_BOILERPLATE(BlockData);
  std::tuple<Statement<BlockDataStmt>, SpecificationPart,
      Statement<EndBlockDataStmt>>
      t;
};

// R1508 generic-spec ->
//         generic-name | OPERATOR ( defined-operator ) |
//         ASSIGNMENT ( = ) | defined-io-generic-spec
// R1509 defined-io-generic-spec ->
//         READ ( FORMATTED ) | READ ( UNFORMATTED ) |
//         WRITE ( FORMATTED ) | WRITE ( UNFORMATTED )
struct GenericSpec {
  UNION_CLASS_BOILERPLATE(GenericSpec);
  EMPTY_CLASS(Assignment);
  EMPTY_CLASS(ReadFormatted);
  EMPTY_CLASS(ReadUnformatted);
  EMPTY_CLASS(WriteFormatted);
  EMPTY_CLASS(WriteUnformatted);
  std::variant<Name, DefinedOperator, Assignment, ReadFormatted,
      ReadUnformatted, WriteFormatted, WriteUnformatted>
      u;
};

// R1510 generic-stmt ->
//         GENERIC [, access-spec] :: generic-spec => specific-procedure-list
struct GenericStmt {
  TUPLE_CLASS_BOILERPLATE(GenericStmt);
  std::tuple<std::optional<AccessSpec>, GenericSpec, std::list<Name>> t;
};

// R1503 interface-stmt -> INTERFACE [generic-spec] | ABSTRACT INTERFACE
struct InterfaceStmt {
  UNION_CLASS_BOILERPLATE(InterfaceStmt);
  std::variant<std::optional<GenericSpec>, Abstract> u;
};

// R1412 only -> generic-spec | only-use-name | rename
// R1413 only-use-name -> use-name
struct Only {
  UNION_CLASS_BOILERPLATE(Only);
  std::variant<Indirection<GenericSpec>, Name, Rename> u;
};

// R1409 use-stmt ->
//         USE [[, module-nature] ::] module-name [, rename-list] |
//         USE [[, module-nature] ::] module-name , ONLY : [only-list]
// R1410 module-nature -> INTRINSIC | NON_INTRINSIC
struct UseStmt {
  BOILERPLATE(UseStmt);
  enum class ModuleNature { Intrinsic, Non_Intrinsic };  // R1410
  template<typename A>
  UseStmt(std::optional<ModuleNature> &&nat, Name &&n, std::list<A> &&x)
    : nature(std::move(nat)), moduleName(std::move(n)), u(std::move(x)) {}
  std::optional<ModuleNature> nature;
  Name moduleName;
  std::variant<std::list<Rename>, std::list<Only>> u;
};

// R1514 proc-attr-spec ->
//         access-spec | proc-language-binding-spec | INTENT ( intent-spec ) |
//         OPTIONAL | POINTER | PROTECTED | SAVE
struct ProcAttrSpec {
  UNION_CLASS_BOILERPLATE(ProcAttrSpec);
  std::variant<AccessSpec, LanguageBindingSpec, IntentSpec, Optional, Pointer,
      Protected, Save>
      u;
};

// R1512 procedure-declaration-stmt ->
//         PROCEDURE ( [proc-interface] ) [[, proc-attr-spec]... ::]
//         proc-decl-list
struct ProcedureDeclarationStmt {
  TUPLE_CLASS_BOILERPLATE(ProcedureDeclarationStmt);
  std::tuple<std::optional<ProcInterface>, std::list<ProcAttrSpec>,
      std::list<ProcDecl>>
      t;
};

// R1527 prefix-spec ->
//         declaration-type-spec | ELEMENTAL | IMPURE | MODULE |
//         NON_RECURSIVE | PURE | RECURSIVE
struct PrefixSpec {
  UNION_CLASS_BOILERPLATE(PrefixSpec);
  EMPTY_CLASS(Elemental);
  EMPTY_CLASS(Impure);
  EMPTY_CLASS(Module);
  EMPTY_CLASS(Non_Recursive);
  EMPTY_CLASS(Pure);
  EMPTY_CLASS(Recursive);
  std::variant<DeclarationTypeSpec, Elemental, Impure, Module, Non_Recursive,
      Pure, Recursive>
      u;
};

// R1532 suffix ->
//         proc-language-binding-spec [RESULT ( result-name )] |
//         RESULT ( result-name ) [proc-language-binding-spec]
struct Suffix {
  BOILERPLATE(Suffix);
  Suffix(LanguageBindingSpec &&lbs, std::optional<Name> &&rn)
    : binding(std::move(lbs)), resultName(std::move(rn)) {}
  Suffix(Name &&rn, std::optional<LanguageBindingSpec> &&lbs)
    : binding(std::move(lbs)), resultName(std::move(rn)) {}
  std::optional<LanguageBindingSpec> binding;
  std::optional<Name> resultName;
};

// R1530 function-stmt ->
//         [prefix] FUNCTION function-name ( [dummy-arg-name-list] ) [suffix]
// R1526 prefix -> prefix-spec [prefix-spec]...
// R1531 dummy-arg-name -> name
struct FunctionStmt {
  TUPLE_CLASS_BOILERPLATE(FunctionStmt);
  std::tuple<std::list<PrefixSpec>, Name, std::list<Name>,
      std::optional<Suffix>>
      t;
};

// R1533 end-function-stmt -> END [FUNCTION [function-name]]
WRAPPER_CLASS(EndFunctionStmt, std::optional<Name>);

// R1536 dummy-arg -> dummy-arg-name | *
struct DummyArg {
  UNION_CLASS_BOILERPLATE(DummyArg);
  std::variant<Name, Star> u;
};

// R1535 subroutine-stmt ->
//         [prefix] SUBROUTINE subroutine-name [( [dummy-arg-list] )
//         [proc-language-binding-spec]]
struct SubroutineStmt {
  TUPLE_CLASS_BOILERPLATE(SubroutineStmt);
  std::tuple<std::list<PrefixSpec>, Name, std::list<DummyArg>,
      std::optional<LanguageBindingSpec>>
      t;
};

// R1537 end-subroutine-stmt -> END [SUBROUTINE [subroutine-name]]
WRAPPER_CLASS(EndSubroutineStmt, std::optional<Name>);

// R1505 interface-body ->
//         function-stmt [specification-part] end-function-stmt |
//         subroutine-stmt [specification-part] end-subroutine-stmt
struct InterfaceBody {
  UNION_CLASS_BOILERPLATE(InterfaceBody);
  struct Function {
    TUPLE_CLASS_BOILERPLATE(Function);
    std::tuple<Statement<FunctionStmt>, Indirection<SpecificationPart>,
        Statement<EndFunctionStmt>>
        t;
  };
  struct Subroutine {
    TUPLE_CLASS_BOILERPLATE(Subroutine);
    std::tuple<Statement<SubroutineStmt>, Indirection<SpecificationPart>,
        Statement<EndSubroutineStmt>>
        t;
  };
  std::variant<Function, Subroutine> u;
};

// R1506 procedure-stmt -> [MODULE] PROCEDURE [::] specific-procedure-list
struct ProcedureStmt {
  enum class Kind { ModuleProcedure, Procedure };
  TUPLE_CLASS_BOILERPLATE(ProcedureStmt);
  std::tuple<Kind, std::list<Name>> t;
};

// R1502 interface-specification -> interface-body | procedure-stmt
struct InterfaceSpecification {
  UNION_CLASS_BOILERPLATE(InterfaceSpecification);
  std::variant<InterfaceBody, Statement<ProcedureStmt>> u;
};

// R1504 end-interface-stmt -> END INTERFACE [generic-spec]
WRAPPER_CLASS(EndInterfaceStmt, std::optional<GenericSpec>);

// R1501 interface-block ->
//         interface-stmt [interface-specification]... end-interface-stmt
struct InterfaceBlock {
  TUPLE_CLASS_BOILERPLATE(InterfaceBlock);
  std::tuple<Statement<InterfaceStmt>, std::list<InterfaceSpecification>,
      Statement<EndInterfaceStmt>>
      t;
};

// R1511 external-stmt -> EXTERNAL [::] external-name-list
WRAPPER_CLASS(ExternalStmt, std::list<Name>);

// R1519 intrinsic-stmt -> INTRINSIC [::] intrinsic-procedure-name-list
WRAPPER_CLASS(IntrinsicStmt, std::list<Name>);

// R1522 procedure-designator ->
//         procedure-name | proc-component-ref | data-ref % binding-name
struct ProcedureDesignator {
  UNION_CLASS_BOILERPLATE(ProcedureDesignator);
  std::variant<Name, ProcComponentRef> u;
};

// R1525 alt-return-spec -> * label
WRAPPER_CLASS(AltReturnSpec, Label);

// R1524 actual-arg ->
//         expr | variable | procedure-name | proc-component-ref |
//         alt-return-spec
struct ActualArg {
  WRAPPER_CLASS(PercentRef, Variable);  // %REF(v) extension
  WRAPPER_CLASS(PercentVal, Expr);  // %VAL(x) extension
  UNION_CLASS_BOILERPLATE(ActualArg);
  ActualArg(Expr &&x) : u{Indirection<Expr>(std::move(x))} {}
  ActualArg(Variable &&x) : u{Indirection<Variable>(std::move(x))} {}
  std::variant<Indirection<Expr>, Indirection<Variable>, Name, ProcComponentRef,
      AltReturnSpec, PercentRef, PercentVal>
      u;
};

// R1523 actual-arg-spec -> [keyword =] actual-arg
struct ActualArgSpec {
  TUPLE_CLASS_BOILERPLATE(ActualArgSpec);
  std::tuple<std::optional<Keyword>, ActualArg> t;
};

// R1520 function-reference -> procedure-designator ( [actual-arg-spec-list] )
struct Call {
  TUPLE_CLASS_BOILERPLATE(Call);
  std::tuple<ProcedureDesignator, std::list<ActualArgSpec>> t;
};
WRAPPER_CLASS(FunctionReference, Call);

// R1521 call-stmt -> CALL procedure-designator [( [actual-arg-spec-list] )]
WRAPPER_CLASS(CallStmt, Call);

// R1529 function-subprogram ->
//         function-stmt [specification-part] [execution-part]
//         [internal-subprogram-part] end-function-stmt
struct FunctionSubprogram {
  TUPLE_CLASS_BOILERPLATE(FunctionSubprogram);
  std::tuple<Statement<FunctionStmt>, SpecificationPart, ExecutionPart,
      std::optional<InternalSubprogramPart>, Statement<EndFunctionStmt>>
      t;
};

// R1534 subroutine-subprogram ->
//         subroutine-stmt [specification-part] [execution-part]
//         [internal-subprogram-part] end-subroutine-stmt
struct SubroutineSubprogram {
  TUPLE_CLASS_BOILERPLATE(SubroutineSubprogram);
  std::tuple<Statement<SubroutineStmt>, SpecificationPart, ExecutionPart,
      std::optional<InternalSubprogramPart>, Statement<EndSubroutineStmt>>
      t;
};

// R1539 mp-subprogram-stmt -> MODULE PROCEDURE procedure-name
WRAPPER_CLASS(MpSubprogramStmt, Name);

// R1540 end-mp-subprogram-stmt -> END [PROCEDURE [procedure-name]]
WRAPPER_CLASS(EndMpSubprogramStmt, std::optional<Name>);

// R1538 separate-module-subprogram ->
//         mp-subprogram-stmt [specification-part] [execution-part]
//         [internal-subprogram-part] end-mp-subprogram-stmt
struct SeparateModuleSubprogram {
  TUPLE_CLASS_BOILERPLATE(SeparateModuleSubprogram);
  std::tuple<Statement<MpSubprogramStmt>, SpecificationPart, ExecutionPart,
      std::optional<InternalSubprogramPart>, Statement<EndMpSubprogramStmt>>
      t;
};

// R1541 entry-stmt -> ENTRY entry-name [( [dummy-arg-list] ) [suffix]]
struct EntryStmt {
  TUPLE_CLASS_BOILERPLATE(EntryStmt);
  std::tuple<Name, std::list<DummyArg>, std::optional<Suffix>> t;
};

// R1542 return-stmt -> RETURN [scalar-int-expr]
WRAPPER_CLASS(ReturnStmt, std::optional<ScalarIntExpr>);

// R1544 stmt-function-stmt ->
//         function-name ( [dummy-arg-name-list] ) = scalar-expr
struct StmtFunctionStmt {
  TUPLE_CLASS_BOILERPLATE(StmtFunctionStmt);
  std::tuple<Name, std::list<Name>, Scalar<Expr>> t;
};

// Extension and deprecated statements
struct BasedPointerStmt {
  TUPLE_CLASS_BOILERPLATE(BasedPointerStmt);
  std::tuple<ObjectName, ObjectName, std::optional<ArraySpec>> t;
};

struct RedimensionStmt {
  TUPLE_CLASS_BOILERPLATE(RedimensionStmt);
  std::tuple<ObjectName, std::list<AllocateShapeSpec>> t;
};

struct Union;
struct StructureDef;

struct StructureField {
  UNION_CLASS_BOILERPLATE(StructureField);
  std::variant<Statement<DataComponentDefStmt>, Indirection<StructureDef>,
      Indirection<Union>>
      u;
};

struct Map {
  EMPTY_CLASS(MapStmt);
  EMPTY_CLASS(EndMapStmt);
  TUPLE_CLASS_BOILERPLATE(Map);
  std::tuple<Statement<MapStmt>, std::list<StructureField>,
      Statement<EndMapStmt>>
      t;
};

struct Union {
  EMPTY_CLASS(UnionStmt);
  EMPTY_CLASS(EndUnionStmt);
  TUPLE_CLASS_BOILERPLATE(Union);
  std::tuple<Statement<UnionStmt>, std::list<Map>, Statement<EndUnionStmt>> t;
};

struct StructureStmt {
  TUPLE_CLASS_BOILERPLATE(StructureStmt);
  std::tuple<Name, std::list<EntityDecl>> t;
};

struct StructureDef {
  EMPTY_CLASS(EndStructureStmt);
  TUPLE_CLASS_BOILERPLATE(StructureDef);
  std::tuple<Statement<StructureStmt>, std::list<StructureField>,
      Statement<EndStructureStmt>>
      t;
};

struct ArithmeticIfStmt {
  TUPLE_CLASS_BOILERPLATE(ArithmeticIfStmt);
  std::tuple<Expr, Label, Label, Label> t;
};

struct AssignStmt {
  TUPLE_CLASS_BOILERPLATE(AssignStmt);
  std::tuple<Label, Name> t;
};

struct AssignedGotoStmt {
  TUPLE_CLASS_BOILERPLATE(AssignedGotoStmt);
  std::tuple<Name, std::list<Label>> t;
};

WRAPPER_CLASS(PauseStmt, std::optional<StopCode>);

// Formatting of template types
// TODO move elsewhere?
template<typename A>
std::ostream &operator<<(std::ostream &o, const Statement<A> &x) {
  return o << "(Statement " << x.label << ' '
           << (x.isLabelInAcceptableField ? ""s : "!isLabelInAcceptableField "s)
           << x.position << ' ' << x.statement << ')';
}

template<typename A>
std::ostream &operator<<(std::ostream &o, const Scalar<A> &x) {
  return o << "(Scalar- " << x.thing << ')';
}

template<typename A>
std::ostream &operator<<(std::ostream &o, const Constant<A> &x) {
  return o << "(Constant- " << x.thing << ')';
}

template<typename A>
std::ostream &operator<<(std::ostream &o, const Integer<A> &x) {
  return o << "(Integer- " << x.thing << ')';
}

template<typename A>
std::ostream &operator<<(std::ostream &o, const Logical<A> &x) {
  return o << "(Logical- " << x.thing << ')';
}

template<typename A>
std::ostream &operator<<(std::ostream &o, const DefaultChar<A> &x) {
  return o << "(DefaultChar- " << x.thing << ')';
}

template<typename A>
std::ostream &operator<<(std::ostream &o, const LoopBounds<A> &x) {
  return o << "(LoopBounds " << x.name << ' ' << x.lower << ' ' << x.upper
           << ' ' << x.step << ')';
}
}  // namespace Fortran
#endif  // FORTRAN_PARSE_TREE_H_