include_directories(. interfaces)

add_executable(space_invaders main.c 
                              hdmi.c 
                              sprites.c 
                              button_debouncer.c 
                              game_over.c 
                              world.c 
                              player.c 
                              globals.c
                              alien_march.c
                              bunkers.c
                              flying_alien.c
                              bullet.c
                              sounds.c)

target_link_libraries(space_invaders buttons switches intc)
