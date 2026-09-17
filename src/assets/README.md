# Assets

Assets are split by semantic role.

## resources/

Reusable building blocks that other assets can reference:

- animation
- font
- image
- midi
- model
- sequence
- sprite
- texture

## content/

Game-authored concepts composed from or referring to resources:

- animation presentation
- floor
- identity kit
- interface
- item
- location
- map
- message
- message animation
- NPC
- parameter
- spot animation
- varbit
- varp

`content/definition/` contains only the shared definition-table format
infrastructure. Individual game domains do not live under a generic
`definition/` umbrella.
