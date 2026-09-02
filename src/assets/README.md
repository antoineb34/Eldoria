# Assets

Source-format parsing and semantic interpretation of game content.

An asset domain may contain its encoded representation, parser, decoder,
semantic representation, repository, and source mapping when those roles are
actually needed.

Assets may depend on Storage/Binary. They must not depend on runtime rendering,
audio playback, application UI, or host integration.
