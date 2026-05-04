---
name: Future AI expansions — AWS Bedrock
description: AI-powered feature expansions for the finance manager app using AWS Bedrock as the backend
type: project
---

User wants to expand the app with AI features powered by AWS Bedrock. All features below are planned for future implementation.

**AI Backend:** AWS Bedrock (note: requires AWS Signature V4 auth in C++, more complex than direct Anthropic API)

## Conversational
- **Finance Chat Assistant** — natural language Q&A over the user's own data ("How much did I spend on food in April?", "Am I over budget?")

## Smart Analysis
- **Spending Insights Panel** — proactive AI analysis of expense history; surfaces trends, anomalies, warnings ("dining spend up 40% vs last month")
- **Monthly Summary Report** — AI-generated natural language recap: top categories, budget performance, notable changes
- **Spending Forecast** — predicts next month's expenses based on historical patterns

## Productivity
- **Natural Language Expense Entry** — type "spent $12 on lunch today", AI parses into structured entry (amount, category, date)
- **Category Auto-Suggest** — AI suggests most likely category from expense description during entry

## Advisory
- **Budget Recommendation Engine** — AI reviews spending history and suggests realistic per-category budget limits
- **Bill Priority Advisor** — given upcoming bills and current balance, AI ranks which to pay first

**How to apply:** When user returns to expand, pick the feature they want, implement libcurl + AWS SigV4 signing + nlohmann/json, then wire into a new ImGui panel. Natural Language Expense Entry and Finance Chat Assistant are the highest day-to-day impact features to prioritize first.
