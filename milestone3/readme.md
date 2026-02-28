# Milestone 3 — Display PNG + Engine Architecture

## 🎯 Goal
Display a PNG image on screen and introduce proper project structure.

---

## 📚 What is implemented
- Image loading (RGBA)
- GPU texture upload (staging buffer)
- Texture sampler
- Descriptor sets
- Graphics pipeline (textured quad or fullscreen triangle)
- Basic blending (for transparency)

---

## 🏗️ Architecture
Project is split into modules:

- Window (SDL only)
- VulkanContext (instance + surface)
- Device (GPU + queues)
- Swapchain (images + views + format)
- Renderer (render pass + commands + sync)
- Assets (textures, loading)

---

## 🧠 What I learned
- Vulkan does not load images (CPU must do it)
- GPU memory vs CPU memory
- Importance of clean architecture early

---

## ✅ Done when
- PNG renders correctly
- Transparency works (if PNG has alpha)
- Resize does not crash the app
- Code is modular and readable

---

## ⚠️ Notes
- This is the transition from "demo" to "engine"
