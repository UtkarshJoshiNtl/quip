# Graph Report - .  (2026-05-28)

## Corpus Check
- Corpus is ~8,773 words - fits in a single context window. You may not need a graph.

## Summary
- 147 nodes · 252 edges · 16 communities (10 shown, 6 thin omitted)
- Extraction: 80% EXTRACTED · 20% INFERRED · 0% AMBIGUOUS · INFERRED: 50 edges (avg confidence: 0.82)
- Token cost: 0 input · 0 output

## Community Hubs (Navigation)
- [[_COMMUNITY_Core Shell Architecture|Core Shell Architecture]]
- [[_COMMUNITY_Main Loop, Signals & Terminal|Main Loop, Signals & Terminal]]
- [[_COMMUNITY_Command Execution & Jobs|Command Execution & Jobs]]
- [[_COMMUNITY_Agent Guide & Build System|Agent Guide & Build System]]
- [[_COMMUNITY_Builtin Commands & History|Builtin Commands & History]]
- [[_COMMUNITY_Configuration System|Configuration System]]
- [[_COMMUNITY_Plugin Client (C)|Plugin Client (C)]]
- [[_COMMUNITY_Python Daemon|Python Daemon]]
- [[_COMMUNITY_VS Code Launch Config|VS Code Launch Config]]
- [[_COMMUNITY_VS Code Build Tasks|VS Code Build Tasks]]
- [[_COMMUNITY_Plugin Design Concepts|Plugin Design Concepts]]
- [[_COMMUNITY_VS Code Launch (design)|VS Code Launch (design)]]
- [[_COMMUNITY_VS Code Build (design)|VS Code Build (design)]]
- [[_COMMUNITY_Screenshot|Screenshot]]

## God Nodes (most connected - your core abstractions)
1. `main()` - 20 edges
2. `Architecture Document` - 16 edges
3. `Design Decisions Document` - 15 edges
4. `execute_command()` - 11 edges
5. `Command Execution Module` - 10 edges
6. `quip shell` - 10 edges
7. `Main Entry Point (REPL Loop)` - 9 edges
8. `Plugin Client Module` - 9 edges
9. `read_line()` - 8 edges
10. `plugin_exec()` - 8 edges

## Surprising Connections (you probably didn't know these)
- `Custom Line Editor vs libreadline` --implements--> `Prompt and Line Editor Module`  [INFERRED]
  DESIGN.md → src/prompt.c
- `Multiline via Backslash and Quote Tracking` --implements--> `Prompt and Line Editor Module`  [INFERRED]
  DESIGN.md → src/prompt.c
- `No Job Control Signals (SIGTSTP Ignored)` --implements--> `Signal Handling Module`  [INFERRED]
  DESIGN.md → src/signals.c
- `Builtin Redirection via fd Save-Restore` --implements--> `Command Execution Module`  [INFERRED]
  DESIGN.md → src/execute.c
- `Fork-Exec Model for External Commands` --implements--> `Command Execution Module`  [INFERRED]
  DESIGN.md → src/execute.c

## Hyperedges (group relationships)
- **Shell Initialization Sequence** — quip_main, quip_config, quip_signals, quip_jobs, quip_history, quip_terminal, quip_prompt, quip_plugin [EXTRACTED 0.95]
- **Three-Stage Command Dispatch Chain** — quip_execute, quip_builtins, quip_plugin [EXTRACTED 0.95]
- **Plugin Subsystem** — quip_plugin, quip_daemon, quip_plugin_hello, quip_plugin_reverse [EXTRACTED 0.90]

## Communities (16 total, 6 thin omitted)

### Community 0 - "Core Shell Architecture"
Cohesion: 0.18
Nodes (27): history_size, Architecture Document, Built-in Commands Module, Tab Completion Module, Configuration Module, Python Plugin Daemon, Abstract Unix Socket for Plugin IPC, Builtin Redirection via fd Save-Restore (+19 more)

### Community 1 - "Main Loop, Signals & Terminal"
Cohesion: 0.14
Nodes (18): builtin_commands_lookup(), count_matches(), find_longest_common_prefix(), handle_completion(), main(), get_command_line(), print_continuation_prompt(), print_prompt() (+10 more)

### Community 2 - "Command Execution & Jobs"
Cohesion: 0.19
Nodes (16): find_builtin(), cleanup_pipes(), execute_command(), execute_line(), execute_pipeline(), handle_redirection(), parse_command_line(), parse_pipeline() (+8 more)

### Community 3 - "Agent Guide & Build System"
Cohesion: 0.16
Nodes (14): Linux abstract Unix socket, execute.c, find_builtin(), fork() + execvp(), gcc, graphify-out/, Length-prefixed JSON IPC, Makefile (+6 more)

### Community 4 - "Builtin Commands & History"
Cohesion: 0.15
Nodes (6): builtin_history(), history_cleanup(), history_get(), history_push(), history_save_to_disk(), history_size()

### Community 5 - "Configuration System"
Cohesion: 0.33
Nodes (9): build_xdg_paths(), config_get(), config_get_int(), config_init(), ensure_dir(), get_config_path(), get_data_path(), history_init() (+1 more)

### Community 6 - "Plugin Client (C)"
Cohesion: 0.40
Nodes (9): json_escape(), plugin_cleanup(), plugin_connect(), plugin_exec(), plugin_init(), plugin_launch_daemon(), plugin_recv(), plugin_send() (+1 more)

### Community 7 - "Python Daemon"
Cohesion: 0.50
Nodes (7): discover_plugin_dirs(), discover_plugins(), handle_exec(), load_plugin_module(), main(), recv_msg(), send_msg()

## Knowledge Gaps
- **14 isolated node(s):** `version`, `configurations`, `tasks`, `version`, `VS Code Launch Configuration` (+9 more)
  These have ≤1 connection - possible missing edges or undocumented components.
- **6 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `main()` connect `Main Loop, Signals & Terminal` to `Command Execution & Jobs`, `Builtin Commands & History`, `Configuration System`, `Plugin Client (C)`?**
  _High betweenness centrality (0.080) - this node is a cross-community bridge._
- **Why does `quip shell` connect `Agent Guide & Build System` to `Core Shell Architecture`?**
  _High betweenness centrality (0.047) - this node is a cross-community bridge._
- **Why does `Configuration Module` connect `Core Shell Architecture` to `Agent Guide & Build System`?**
  _High betweenness centrality (0.028) - this node is a cross-community bridge._
- **Are the 19 inferred relationships involving `main()` (e.g. with `config_init()` and `execute_line()`) actually correct?**
  _`main()` has 19 INFERRED edges - model-reasoned connections that need verification._
- **Are the 6 inferred relationships involving `execute_command()` (e.g. with `find_builtin()` and `add_job()`) actually correct?**
  _`execute_command()` has 6 INFERRED edges - model-reasoned connections that need verification._
- **Are the 3 inferred relationships involving `Command Execution Module` (e.g. with `Builtin Redirection via fd Save-Restore` and `Fork-Exec Model for External Commands`) actually correct?**
  _`Command Execution Module` has 3 INFERRED edges - model-reasoned connections that need verification._
- **What connects `version`, `configurations`, `tasks` to the rest of the system?**
  _14 weakly-connected nodes found - possible documentation gaps or missing edges._