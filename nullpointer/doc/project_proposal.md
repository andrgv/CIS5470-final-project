# Detecting Integer Overflow and Null-pointer Dereferencing

Target bug classes (2–3) with rationale drawn from Labs 1–7.

    Integer overflow and precision loss

    Null-pointer dereference

 Technique plan: for each bug class, identify the analysis paradigm(s) you will employ (e.g., RedQueen-inspired fuzzing, abstract interpretation with narrowing over intervals, solver-aided symbolic execution) and the key technical challenges you expect.

    Integer overflow and precision loss: 

        Techniques: Abstract Interpretation and Dataflow Extension. Will implement forward intraprocedural dataflow analysis. For each integer value we will track range of the Interval Domain and update them by widening or narrowing them as required. For example, at the start of loops we want to widen and the narrow for precision. This will show whether it might overflow. Precision loss will be determined when casts truncate a value or lose precision in another way. 

        Implementing the widening and narrowing strategy with loop boundaries will be a challenge since we might become too imprecise or we might grow the boundaries forever and never converge.

    Null-pointer dereference

        Techniques: dataflow analysis, flow-insensitive/flow-sensitive points-to analysis.

        We are going to implement dataflow analysis similar to labs 6/7, where we will track variables with NotNull, MaybeNull, and Null domains. We will perform chaotic iteration, repeatedly applying transfer functions. When a dereference is performed on a variable with domain MaybeNull or Null, we may have a null pointer dereference.

        One of the key technical challenges we expect is handling pointer analysis, where assignments to one pointer location propagate to aliases. Another challenge might be extending to handle interprocedural analysis, in particular properly propagating nullness across parameters and return values.

     Benchmark plan: describe existing suites you will reuse or new programs you will craft, including program sizes and how you will establish ground truth.

    We will create several basic test cases involving simple functions and one clear error point. We will then create a few test cases involving conditionals. In the case of null pointer dereferencing, we will create several test cases to test aliasing and some test cases involving multiple functions. These test cases can be constructed taking inspiration from the test suites in labs 6 and 7 but adapted for this new bug class.

    We will look at and test some cases from https://cwe.mitre.org/data/definitions/190.html for integer overflow or wraparound and https://cwe.mitre.org/data/definitions/476.html for null pointer dereference

     Implementation tasks and milestones.

        Create proposal and set up repository for work

        Write test cases

        Interval testing MVP: Implement interval domain for integers

        Null pointer dereference MVP: Implement transfer functions, chaotic iteration, etc to detect null pointer dereferencing in simple programs

        Null pointer dereference stretch: Implement pointer analysis and handle additional function calls

        Evaluation and improving accuracy loop

        Write report draft

     Partial-success criteria (what you need for a meaningful result even if stretch goals slip).

    For integer overflow and precision loss: be able to detect some of the potential overflows

    For null pointer dereference: be able to detect all potential null pointer dereferences in simple programs and programs involving conditionals

    Core analysis runs, some integer overflows and potential null pointer errors are detected. Have some tests to run.

     Full-success criteria (what “done” looks like, including evaluation metrics).

    For integer overflow and precision loss: be able to detect potential overflows and underflows

    For null pointer dereference: be able to detect potential null pointer dereferences in programs involving pointer aliasing and function calls.

     Risk assessment and contingency plans.

    If encounter issues in pointer analysis/function call stretch goals: focus on only one and just try to pass as many tests as possible, while dropping the other goal. This

     Work division between teammates.

    While we plan on working together on most of the components and integration of the project, one of us will become more specialized in the integer overflow analysis and the other in the null pointer analysis.