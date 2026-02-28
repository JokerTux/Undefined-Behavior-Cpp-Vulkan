# Milestone 2 — First Vulkan Frame (Clear Color)

## 🎯 Goal
Render a solid color to the screen using Vulkan.

---

## 📚 What is implemented
- Physical device selection
- Logical device + queues
- Swapchain creation
- Image views
- Render pass
- Framebuffers
- Command pool + command buffers
- Synchronization (semaphores + fences)
- Frame loop: acquire → submit → present

---

## 🧠 What I learned
- Swapchain is the core of presentation
- Vulkan rendering requires multiple steps
- Synchronization is mandatory
- GPU and CPU run asynchronously

---

## ✅ Done when
- Screen is cleared with a solid color every frame
- No flickering
- No validation errors
- Application runs stable

---

## ⚠️ Notes
- No textures or images yet
- This is the first "it works" Vulkan moment
