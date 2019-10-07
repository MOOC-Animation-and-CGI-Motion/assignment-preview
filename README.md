# Assignment Preview

This assignment preview is designed for all learners (both audit track and verified track) to practice the course' six assignments in a local virtual machine. This allows you to do the projects in an ungraded, untimed environment. These assignments cover the following topics:

- Integrators (Explicit Euler, Symplectic Euler, Implicit Euler)
- Basic Physic Forces (Gravity, Mass Spring systems, Linear Drag)
- Collisions (Particles, Edges, Planes)

These assignments are designed to give you a taste of what the projects will be like. However, please note that they are UNGRADED. These assignments only cover a portion of the assignments available to Verified Learners. Topics that are not covered by these assignments include:

- Rigidbodies
- Softbodies
- Fluids

These assignments can serve as both preview material (for students deciding whether or not they want to take the course) as well as review material.

For verified track learners, you can use it to practice and review for the final exam in your local machine.

Please read the instructions below carefully to set up a local Ubuntu Virtual Machine (VM) and download the starter code for the first 6 assignments.

Please note: Our staff are not responsible for incompatibility between the local Virtual Machine and Codio.

# Instructions for Local VM

Please Note: the instructions are the same for Mac/Windows/Linux

1. Download and install Oracle VM VirtualBox from https://www.virtualbox.org/wiki/Downloads
2. Download an Ubuntu image by navigating to this [link](https://ubuntu.com/download/alternative-downloads) and scroll down to the BitTorrent section and select the [Ubuntu 16.04.5 Desktop (64-bit)](http://releases.ubuntu.com/16.04/ubuntu-16.04.6-desktop-amd64.iso.torrent?_ga=2.159199036.782935894.1570480264-842531798.1570480264) option.
3. Follow [these instructions](https://itsfoss.com/install-linux-in-virtualbox/) to install Ubuntu on VirtualBox
4. After Ubuntu is installed, clone this repo.

```
git clone https://github.com/MOOC-Animation-and-CGI-Motion/assignment-preview.git
```

5. Install dependencies

```
sudo apt-get install freeglut3 freeglut3-dev cmake
```

Now you can start running the program!

# FAQ

If installation of Ubuntu fails, then try:

- Assigning more memory and storage
- Go to BIOS and check that virtualization technology is enabled (Windows only)

If display of VM is too small then you can scale the VM display

# Note for the learners

You can follow the instructions here to setup the simulator on your own machine. We are providing all the files you need to run the simulator and the first six assignments. The simulator comes with a test software that lets you run your code against and compare your results. However, testing done by the software in not exhaustive and passing all the tests don’t indicate a perfect score as there might be edge cases not accounted by the software. The purpose of this is to let you play with the engine as you wish and work on your code outside of the Codio platform. Consequently, feedback and debugging help by the staff won’t be provided for code outside of the Codio platform.
