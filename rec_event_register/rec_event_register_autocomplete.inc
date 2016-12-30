<?php
/**
 * @file
 *   Registration Administration autocomplete file
 *
 * @author
 *   gkarak@forecasting.gr
 *
 * @copyright
 *   (c) 2014 George Karakostas
 */

/**
 * Implements autocomplete function
 *
 * @see http://www.coalmarch.com/blog/autocomplete-drupal-form-tutorial
 * @see https://api.drupal.org/api/drupal/modules%21user%21user.pages.inc/function/user_autocomplete/7
 *
 * @param string $string
 */
function rec_event_register_autocomplete($string = '') {
  // look for a name field in users (name module)
  // @see https://api.drupal.org/api/drupal/modules!field!field.info.inc/function/field_info_instances/7
  $field = '';
  $instances = field_info_instances('user');
  foreach($instances['user'] as $field_name => $field_inst) {
    if ($field_inst['widget']['module'] == 'name') $field = $field_name;
  }
  // init
  $matches = array();
  // if name field exists
  if ($string && $field) {
    $query = db_select('users', 'u');
    $query->join('field_data_field_name', 'n',
      // place this condition in join and not where
      // so that if a user does not
      // have name to still allow search
      "u.uid = n.entity_id AND n.entity_type = 'user'");
    $query
      ->fields('u', array('name'))
      ->fields('n', array(
        'field_name_given',
        'field_name_middle',
        'field_name_family',
      ))
      ->condition('u.status', 1)
      ->condition(db_or()
          ->condition('u.name', db_like($string) . '%', 'LIKE')
          ->condition('n.field_name_given', db_like($string) . '%', 'LIKE')
          ->condition('n.field_name_middle', db_like($string) . '%', 'LIKE')
          ->condition('n.field_name_family', db_like($string) . '%', 'LIKE')
      )
      ->range(0, 10);
    $result = $query->execute();
    //dpq($result);
    foreach ($result as $user) {
      // @see http://stackoverflow.com/a/5985436/940098
      $matches[$user->name] = check_plain(implode(' ', array_filter(array(
        $user->field_name_given,
        $user->field_name_middle,
        $user->field_name_family,
        '[' . $user->name . ']',
      ))));
    }
  }
  // else no field name, default user autocomplete
  elseif ($string) {
    $result = db_select('users')
      ->fields('users', array('name'))
      ->condition('name', db_like($string) . '%', 'LIKE')
      ->range(0, 10)
      ->execute();
    foreach ($result as $user) {
      $matches[$user->name] = check_plain($user->name);
    }
  }
  drupal_json_output($matches);
}