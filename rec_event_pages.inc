<?php
/**
 * @file
 *   Entity page file
 * 
 * @author
 *   gkarak@forecasting.gr
 *   
 * @copyright
 *   (c) 2014 George Karakostas
 */

/**
 * View callback.
 */
function rec_event_view($event) {
  drupal_set_title(entity_label('rec_event', $event));
  return entity_view('rec_event', array(entity_id('rec_event', $event) => $event), 'full');
}